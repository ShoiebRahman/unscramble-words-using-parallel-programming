#include <mpi.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

/*partition function for quicksort*/
int partition (char arr[], int l,int h)
{
    int i,j;
    char temp;
    char pivot=arr[h];
    i=l-1;
    for(j=l;j<h;j++)
    {
        if((arr[j])<(pivot))
        {
            i++;
            temp=arr[j];
            arr[j]=arr[i];
            arr[i]=temp;

        }
    }
    temp=arr[h];
    arr[h]=arr[i+1];
    arr[i+1]=temp;
    return (i+1);
}

/*main quicksort function*/
void quicksort(char arr[],int f,int l)
{
    int p;
    if(f<l)
    {
        p=partition(arr,f,l);
        quicksort(arr,f,p-1);
        quicksort(arr,p+1,l);
    }
}

int main(int argc, char** argv) {

    long words_count,ptr_pos,i,j, count, word, jumble_count;
    int block_count;
    int local_char_count;
    int world_rank;
    int world_size;
    int dest;
    int normal_block = 1;
    int last_block = 2;
    int no_work = 3;
    int work_stat;
    char* word_list;
    char* jumbled_list_orignal;
    char* jumbled_list_sort;
    char* block_word_list;
    char* sorted;
    int* block_start;
    int* match_found;
    int* res_match_found;
    long character_count;
    long jumble_character_count;
    double t1,t2;
    char filename[20];
    FILE *fptr;
    FILE *jumbleTxt;
    FILE *resultTxt;

    int block_size;
    int final_block_size;
    int total_blocks;

    t1=MPI_Wtime();

    fptr = fopen("words1.txt","r");
    jumbleTxt = fopen("jumbled.txt","r");


    if (fptr == NULL) {
        printf("Error opening file ! -> fptr\n");
        return 0;  /*Exit with an error code*/
    }

    if (jumbleTxt == NULL) {
        printf("Error opening file ! -> jumbleTxt\n");
        return 0;  /*Exit with an error code*/
    }

    if(fscanf(fptr,"%ld",&words_count)!=1){
        printf("error reading");
    }

    if(fscanf(jumbleTxt,"%ld",&jumble_count)!=1){
        printf("error reading");
    }

    ptr_pos = ftell(jumbleTxt);
    fseek(jumbleTxt,0,SEEK_END);
    jumble_character_count = ftell(jumbleTxt)-ptr_pos-1;
    fseek(jumbleTxt,ptr_pos,SEEK_SET);

    jumbled_list_orignal = (char*)malloc((jumble_character_count+1)*sizeof(char));

    word=0;
    count=0;
    block_count=0;

    while(word<jumble_count){
        if (fscanf(jumbleTxt,"%s", &jumbled_list_orignal[count]) != 1) {
            printf("error reading");
            return 0;
        }
        count += strlen(&jumbled_list_orignal[count]) + 1;
        word=word+1;
    }

    jumbled_list_sort = (char*)malloc((jumble_character_count+1)*sizeof(char));

    memcpy(jumbled_list_sort,jumbled_list_orignal,(jumble_character_count+1)*sizeof(char));

    word=0;
    count=0;
    while(word<jumble_count){
        quicksort(jumbled_list_sort,count,count+strlen(&jumbled_list_sort[count])-1);
        count=count+strlen(&jumbled_list_sort[count])+1;
        word=word+1;
    }

    match_found = (int*)malloc(jumble_count*sizeof(int));
    for(i=0;i<jumble_count;i++){
        match_found[i]=0;
    }

    block_size = 1000;
    total_blocks = (words_count/block_size)+1;
    final_block_size = words_count - block_size*(total_blocks - 1) ;
    

    MPI_Init(NULL, NULL);

    MPI_Status status;

    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    

    if (world_size < 2) {
        fprintf(stderr, "World size must be greater than 1 for %s\n", argv[0]);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    if (world_rank == 0) {

        ptr_pos = ftell(fptr);
        fseek(fptr,0,SEEK_END);
        character_count = ftell(fptr)-ptr_pos-1;
        fseek(fptr,ptr_pos,SEEK_SET);

        printf("\n\n%ld\n\n",character_count);
        printf("\n\n%ld\n\n",words_count);

        word_list = (char*)malloc((character_count+1)*sizeof(char));
        block_start = (int*)malloc(total_blocks*sizeof(int));
        res_match_found = (int*)malloc(jumble_count*sizeof(int));

        for(i=0;i<jumble_count;i++){
            res_match_found[i]=0;
        }

        word=0;
        count=0;
        block_count=0;

        resultTxt = fopen("result/WordNotFound.txt","w");

        while(word<words_count){
            if (fscanf(fptr,"%s", &word_list[count]) != 1) {
                printf("error reading");
                MPI_Abort(MPI_COMM_WORLD, 1);
            }
            
            if(word%block_size == 0){
                block_start[block_count] = count;
                block_count++;
            }
            count += strlen(&word_list[count]) + 1;
            word=word+1;

        }
        

        for(i=0;i<total_blocks;i++){
            if(i==total_blocks-1){
                MPI_Recv(&dest, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&last_block, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
                MPI_Send(&word_list[block_start[i]], count-block_start[i], MPI_CHAR, dest, 0, MPI_COMM_WORLD);

                for(j=1;j<world_size-1;j++){
                    MPI_Recv(&dest, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                    MPI_Send(&no_work, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
                }
            }
            else{
                MPI_Recv(&dest, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                MPI_Send(&normal_block, 1, MPI_INT, dest, 0, MPI_COMM_WORLD);
                MPI_Send(&word_list[block_start[i]], block_start[i+1]-block_start[i], MPI_CHAR, dest, 0, MPI_COMM_WORLD);
            }
        }

        MPI_Reduce(match_found,res_match_found,jumble_count,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
        fprintf(resultTxt,"\nJumbled words not found:\n");
        /*printf("\n\nJumbled words not found:\n\n");*/
        for(i=0,j=0;i<jumble_count;i++,j+=strlen(&jumbled_list_orignal[j])+1)
        {
            if(res_match_found[i]==0){
                fprintf(resultTxt,"%s\n",&jumbled_list_orignal[j]);
                /*printf("\n\n%s\n\n",&jumbled_list_orignal[j]);*/
            }
        }

        t2=MPI_Wtime();
        printf("\n\n%f\n\n",t2-t1);

        free(word_list);
        free(block_start);
        fclose(resultTxt);
        free(res_match_found);

    } 
    else{
        sprintf(filename, "result/process%d.txt", world_rank);
        resultTxt = fopen(filename,"w");
        

        work_stat=1;
        while(1){
            MPI_Send(&world_rank, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
            MPI_Recv(&work_stat, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            if(work_stat==1 || work_stat==2){
                MPI_Probe(0,0,MPI_COMM_WORLD,&status);
                MPI_Get_count(&status, MPI_CHAR, &local_char_count);
                block_word_list = (char*)malloc((local_char_count+1)*sizeof(char));
                sorted = (char*)malloc((local_char_count+1)*sizeof(char));
                MPI_Recv(block_word_list, local_char_count, MPI_CHAR, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                memcpy(sorted,block_word_list,(local_char_count)*sizeof(char));
                word=0;
                count=0;
                if(work_stat==1){
                    words_count = block_size;
                }
                else{
                    words_count= final_block_size;
                }
                while(word<words_count){
                    quicksort(sorted,count,count+strlen(&sorted[count])-1);
                    for(i=0,j=0;i<jumble_count;i++,j+=strlen(&jumbled_list_sort[j])+1)
                    {
                        if(strcmp(&sorted[count],&jumbled_list_sort[j])==0){
                            fprintf(resultTxt,"%s -> %s\n",&jumbled_list_orignal[j],&block_word_list[count]);
                            /*printf("\n\n%s -> %s\n\n",&jumbled_list_orignal[j],&block_word_list[count]);*/
                            match_found[i]=match_found[i]+1;
                        }
                    }
                    /*   printf("%s\n", &sorted[count]);  */
                    count=count+strlen(&sorted[count])+1;
                    word=word+1;
                }
                free(block_word_list);
                free(sorted);
                if(work_stat == 2){
                    break;
                }
            }
            else if(work_stat == 3){
                break;
            }
        }

        MPI_Reduce(match_found,res_match_found,jumble_count,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
        fclose(resultTxt);
    }
    free(match_found);
    MPI_Finalize();

    free(jumbled_list_orignal);
    free(jumbled_list_sort);
    fclose(fptr);
    fclose(jumbleTxt);
    return 0;
}
