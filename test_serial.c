#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Partition function for quicksort */
int partition(char arr[], int l, int h) {
    int i, j;
    char temp;
    char pivot = arr[h];
    i = l - 1;
    for (j = l; j < h; j++) {
        if (arr[j] < pivot) {
            i++;
            temp = arr[j];
            arr[j] = arr[i];
            arr[i] = temp;
        }
    }
    temp = arr[h];
    arr[h] = arr[i + 1];
    arr[i + 1] = temp;
    return (i + 1);
}

/* Main quicksort function */
void quicksort(char arr[], int f, int l) {
    int p;
    if (f < l) {
        p = partition(arr, f, l);
        quicksort(arr, f, p - 1);
        quicksort(arr, p + 1, l);
    }
}

int main() {
    long words_count, ptr_pos, i, j, count, word, jumble_count;
    char* word_list;
    char* jumbled_list_original;
    char* jumbled_list_sort;
    FILE* fptr;
    FILE* jumbleTxt;
    FILE* resultTxt;

    fptr = fopen("words1.txt", "r");
    jumbleTxt = fopen("jumbled.txt", "r");
    printf("\nSTARTED RUNNING");
    if (fptr == NULL || jumbleTxt == NULL) {
        printf("Error opening files!\n");
        return 1; // Exit with an error code
    }

    if (fscanf(fptr, "%ld", &words_count) != 1 || fscanf(jumbleTxt, "%ld", &jumble_count) != 1) {
        printf("Error reading word counts!\n");
        return 1;
    }
    printf("\nSTAGE 1");

    // Allocate memory for jumbled lists
    jumbled_list_original = (char*)malloc((jumble_count + 1) * sizeof(char));
    jumbled_list_sort = (char*)malloc((jumble_count + 1) * sizeof(char));

    
    if (jumbled_list_original == NULL || jumbled_list_sort == NULL) {
        printf("Memory allocation failed!\n");
        return 1;
    }

    printf("\nMEM ALLOC DONE");
    
    // Read jumbled words
    word = 0;
    count = 0;
    int flag = 0;
    printf("\njumbled_count: %d", jumble_count);
    
    while (word < jumble_count) {
        /*if (fscanf(jumbleTxt, "%s", &jumbled_list_original[count]) != 1) {
            printf("\nError reading jumbled words!\n");
            return 1;
        } */
        if (fgets(&jumbled_list_original[count], 100, jumbleTxt) == NULL) {
            printf("Error reading jumbled words!\n");
            return 1;
        }
        //count += strlen(&jumbled_list_original[count]) + 1;
        //jumbled_list_original[count][strcspn(jumbled_list_original[count], "\n")] = '\0';
        
        //count += strlen(&jumbled_list_original[count]) + 1;
        if (count >= (jumble_count + 1) * sizeof(char)) {
            printf("\nCount exceeds allocated memory for jumbled words! - %d\n", count);
            return 1;
        }
        // Move count to the next word
        count++;
        word++;
    }
    /*/
    while (word < jumble_count) {
        //printf("\nFLAG: %d", flag);
        //flag++;
        if (fscanf(jumbleTxt, "%s", &jumbled_list_original[count]) != 1) {
            printf("Error reading jumbled words!\n");
            return 1;
        }
        count += strlen(&jumbled_list_original[count]) + 1;
        word++;
    }
    */
    printf("\nword: %d", word);
    printf("\ncount: %d", count);

    printf("\nSTAGE 2");

    // Copy original jumbled list for sorting
    memcpy(jumbled_list_sort, jumbled_list_original, (jumble_count + 1) * sizeof(char));

    // Sort jumbled words
    word = 0;
    count = 0;
    while (word < jumble_count) {
        quicksort(jumbled_list_sort, count, count + strlen(&jumbled_list_sort[count]) - 1);
        count = count + strlen(&jumbled_list_sort[count]) + 1;
        word++;
    }

    printf("\nSTAGE 3");

    // Allocate memory for word list
    //word_list = (char*)malloc((words_count + 1) * sizeof(char));
    // Allocate memory for word list
    word_list = (char*)malloc((words_count * 100 + 1) * sizeof(char)); // Assuming maximum word length of 100 characters
    printf("Expected buffer size: %ld\n", (words_count * 100 + 1) * sizeof(char));
    printf("Words count: %ld\n", words_count);


    // Read words
    word = 0;
    count = 0;
    fseek(fptr, 0, SEEK_SET);
    
    /*while (word < words_count) {
        if (fscanf(fptr, "%s", &word_list[count]) != 1) {
            printf("Error reading words!\n");
            return 1;
        }
        count += strlen(&word_list[count]) + 1;
        word++;
    } */
    
    while (word < words_count) {
    if (fgets(&word_list[count], 100, fptr) == NULL) { // Assuming maximum word length of 100 characters
        printf("Error reading words!\n");
        return 1;
    }
    count += strlen(&word_list[count]) + 1;
    word++;
    // Check if we reached the end of file prematurely
    if (feof(fptr)) {
        printf("Premature end of file!\n");
        return 1;
    }
    // Check for buffer overflow
    if (count >= (words_count + 1) * sizeof(char)) {
        printf("Buffer overflow!\n");
        return 1;
    }
}

    printf("\nSTAGE 4");

    // Open result file for writing
    resultTxt = fopen("result.txt", "w");
    if (resultTxt == NULL) {
        printf("Error opening result file!\n");
        return 1;
    }

    // Match jumbled words with word list and write results
    for (i = 0; i < jumble_count; i++) {
        int found = 0;
        for (j = 0; j < words_count; j++) {
            char temp[strlen(&jumbled_list_original[i]) + 1];
            strcpy(temp, &word_list[j]);
            quicksort(temp, 0, strlen(temp) - 1);
            if (strcmp(temp, &jumbled_list_sort[i]) == 0) {
                fprintf(resultTxt, "%s -> %s\n", &jumbled_list_original[i], &word_list[j]);
                found = 1;
                break;
            }
        }
        if (!found) {
            fprintf(resultTxt, "%s -> Not Found\n", &jumbled_list_original[i]);
        }
    }

    printf("\nDONE");
    // Cleanup
    free(word_list);
    free(jumbled_list_original);
    free(jumbled_list_sort);
    fclose(fptr);
    fclose(jumbleTxt);
    fclose(resultTxt);

    return 0;
}
