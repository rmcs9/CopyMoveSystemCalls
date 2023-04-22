/**
 * ICSI 333 System Fundamentals
 * Spring 2023
 * Manasa Nannuru
 * Ryan McSweeney
 * RM483514
 * 001521860
 */

#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/file.h>
#include <stdlib.h>
int main(int argc, char **argv) {
    int destinationIndex;
    //this loop determines the integer index that the destination directory or location is stored in
    for (int i = 0; argv[i] != NULL; i++) {
        if (argv[i + 1] == NULL) {
            destinationIndex = i;
        }
    }
    //there must be more than 3 arguments to run the programs
    if (argc >= 3) {
        struct stat buf;
        //COPY BRANCH
        if (strcmp(argv[0], "./copy") == 0) {
            printf("you requested a copy\n");
            //checks if the destination is valid(DIRECTORY/DEVICE)
            stat(argv[destinationIndex], &buf);
            if ((S_ISDIR(buf.st_mode)) == 0 && (S_ISBLK(buf.st_mode)) == 0 && (S_ISCHR(buf.st_mode)) == 0) {
                fprintf(stderr, "to copy a file, destination must be either a directory, or a device\n");
                return 0;
            }
            struct stat fileCheck;
            //loops over src arguments
            for (int i = 1; i < destinationIndex; i++) {
                int success = 1;
                //checks if the inputted src is a regular file
                stat(argv[i], &fileCheck);
                if (S_ISREG(fileCheck.st_mode)) {
                    //if it is, file is opened for reading
                    int srcFileDescrip = open(argv[i], O_RDONLY, S_IRWXU);
                    if(srcFileDescrip != -1){
                        //if the src is inputted as a path then this block finds the name at the end
                        char *filename = strrchr(argv[i], '/');
                        if (filename != NULL) {
                            filename++;
                        }
                        else{
                            filename = argv[i];
                        }

                        struct stat overWriteCheck;
                        char buffer[BUFSIZ];
                        size_t bytes;
                        //creates a new string variable for the destination path of the file being copied
                        char *destString = malloc(strlen(argv[destinationIndex]) + strlen(argv[i]));
                        strcpy(destString, argv[destinationIndex]);
                        strcat(destString, filename);
                        if ((stat(destString, &overWriteCheck)) == 0) {
                            //file already exists inside destination directory
                            //permission to overwrite required
                            printf("file with name %s has been found in destination directory. Would you like to overwrite this file? (y/n)\n",
                                   argv[i]);
                            char yn = '\0';
                            scanf(" %c", &yn);
                            if (yn == 'y' || yn == 'Y') {
                                printf("overwriting file...\n");
                                int destinationFileDescrip = open(destString, O_WRONLY | O_TRUNC, S_IRWXU);
                                if (destinationFileDescrip != -1) {
                                    //WRITE HERE
                                    while((bytes = read(srcFileDescrip, buffer, BUFSIZ)) > 0){
                                        if(write(destinationFileDescrip, buffer, bytes) != bytes){
                                            fprintf(stderr, "failed to write output to file %s\n", filename);
                                            success = 0;
                                        }
                                    }
                                    
                                } else {
                                    fprintf(stderr, "could not open file %s at location %s\n", filename,
                                            argv[destinationIndex]);
                                    success = 0;
                                }
                            } else if(yn == 'n' || yn == 'N'){
                                printf("file has not been overwritten, moving on to next file...\n");
                            }
                        } else {
                            //file does not exist inside destination dir
                            //free to copy file
                            int destinationFileDescrip = open(destString, O_WRONLY | O_CREAT, S_IRWXU);
                            if(destinationFileDescrip != -1){
                                //WRITE HERE
                                while((bytes = read(srcFileDescrip, buffer, BUFSIZ)) > 0){
                                    if(write(destinationFileDescrip, buffer, bytes) != bytes){
                                        fprintf(stderr, "failed to write output to file %s\n", filename);
                                        success = 0;
                                    }
                                }
                            }
                            else{
                                fprintf(stderr, "could not open file %s at location %s\n", filename,
                                        argv[destinationIndex]);
                                success = 0;
                            }

                            if(success){
                                printf("successfully copied %s to %s\n", filename, argv[destinationIndex]);
                            }
                        }
                    }
                    else{
                        printf("could not access file at %s. moving on to next file\n", argv[i]);
                    }
                } else {
                    fprintf(stderr, "file %s does not exist or cannot be accessed. trying next file\n", argv[i]);
                }
            }
        //MOVE BRANCH
        } else if (strcmp(argv[0], "./move") == 0) {
            printf("you requested a move\n");
            stat(argv[destinationIndex], &buf);
            if ((S_ISDIR(buf.st_mode)) == 0) {
                fprintf(stderr, "to move a file, file destination must be a directory\n");
                return 0;
            }

            struct stat fileCheck;
            for (int i = 1; i < destinationIndex; i++) {
                //this int turns to 0 if moving was unsuccessfull
                int success = 1;
                stat(argv[i], &fileCheck);
                if (S_ISREG(fileCheck.st_mode)) {
                    //THIS IS WHERE MOVING TAKES PLACE
                    //READ FROM THE FILE THEN CREATE A NEW FILE AT DESTINATION, WRITE TO IT, THEN MAKE A NEW LINK TO IT AND UNLINK OLD LINK
                    //if the src is inputted as a path then this block finds the name at the end
                    char *filename = strrchr(argv[i], '/');
                    if (filename != NULL) {
                        filename++;
                    }
                    else{
                        filename = argv[i];
                    }

                    //creates a new string variable for the destination path of the file being copied
                    char *destString = malloc(strlen(argv[destinationIndex]) + strlen(argv[i]));
                    strcpy(destString, argv[destinationIndex]);
                    strcat(destString, filename);
                    //incase file fails to link, manual copying is done
                    if(link(argv[i], destString) < 0){
                        printf("cant link %s to destination %s\n", filename, destString);
                        perror("link\n");
                        printf("manual copy and deletion will now occur\n");

                        int srcFileDescrip = open(argv[i], O_RDONLY, S_IRWXU);
                        struct stat overWriteCheck;
                        char buffer[BUFSIZ];
                        size_t bytes;
                        //creates a new string variable for the destination path of the file being copied
                        char *destString = malloc(strlen(argv[destinationIndex]) + strlen(argv[i]));
                        strcpy(destString, argv[destinationIndex]);
                        strcat(destString, filename);
                        if ((stat(destString, &overWriteCheck)) == 0) {
                            //file already exists inside destination directory
                            //permission to overwrite required
                            printf("file with name %s has been found in destination directory. Would you like to overwrite this file? (y/n)\n",
                                   argv[i]);
                            char yn = '\0';
                            scanf(" %c", &yn);
                            if (yn == 'y' || yn == 'Y') {
                                printf("overwriting file...\n");
                                int destinationFileDescrip = open(destString, O_WRONLY | O_TRUNC, S_IRWXU);
                                if (destinationFileDescrip != -1) {
                                    //WRITE HERE
                                    while((bytes = read(srcFileDescrip, buffer, BUFSIZ)) > 0){
                                        if(write(destinationFileDescrip, buffer, bytes) != bytes){
                                            fprintf(stderr, "failed to write output to file %s\n", filename);
                                            success = 0;
                                        }
                                    }

                                } else {
                                    fprintf(stderr, "could not open file %s at location %s\n", filename,
                                            argv[destinationIndex]);
                                    success = 0;
                                }
                            } else if(yn == 'n' || yn == 'N'){
                                printf("file has not been overwritten, moving on to next file...\n");
                            }
                        } else {
                            //file does not exist inside destination dir
                            //free to copy file
                            int destinationFileDescrip = open(destString, O_WRONLY | O_CREAT, S_IRWXU);
                            if(destinationFileDescrip != -1){
                                //WRITE HERE
                                while((bytes = read(srcFileDescrip, buffer, BUFSIZ)) > 0){
                                    if(write(destinationFileDescrip, buffer, bytes) != bytes){
                                        fprintf(stderr, "failed to write output to file %s\n", filename);
                                        success = 0;
                                    }
                                }
                            }
                            else{
                                fprintf(stderr, "could not open file %s at location %s\n", filename,
                                        argv[destinationIndex]);
                                success = 0;
                            }
                        }
                    }
                    //file unlinks here
                    if(success) {
                        if (unlink(argv[i]) < 0) {
                            fprintf(stderr, "failed to unlink at %s\n", argv[i]);
                            success = 0;
                        }
                    }

                    if(success){
                        printf("successfully moved %s to %s\n", filename, argv[destinationIndex]);
                    }
                } else {
                    fprintf(stderr, "file %s does not exist or cannot be accessed. trying next file\n", argv[i]);
                }
            }
        }
    } else {
        fprintf(stderr, "program requires at least 1 source file and 1 destination path\n");
    }
}
