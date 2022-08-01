#include "gaz_cam.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
void randMat(int** matrix_temp){//[ ][SNAPSHOT_WIDTH]
    int i,j,new_temp,height_pos,width_pos,num_iterations=0.1*SNAPSHOT_WIDTH*SNAPSHOT_HEIGHT;
    for(i=0;i<SNAPSHOT_HEIGHT;i++){
        for(j=0;j<SNAPSHOT_WIDTH;j++){
            matrix_temp[i][j]=27;
        }
    }
    for(i=0;i<num_iterations;i++)
    {
        height_pos=rand() % SNAPSHOT_HEIGHT;
        width_pos=rand() % SNAPSHOT_WIDTH;
        new_temp=rand()%(MAX_TEMP+1);
        matrix_temp[height_pos][width_pos]=new_temp;
    }
}
void covert_to_rgb(handler * handle,char rgb_matrix[],int** matrix){//[SNAPSHOT_HEIGHT][SNAPSHOT_WIDTH] [SNAPSHOT_HEIGHT][SNAPSHOT_WIDTH]
    int k=0;
    for(int i=0;i<SNAPSHOT_HEIGHT;i++)
        for(int j=0;j<SNAPSHOT_WIDTH;j++)
        {
            int t=matrix[i][j]*3;
            rgb_matrix[k++]=handle->static_mat_rgb[t];
            rgb_matrix[k++]=handle->static_mat_rgb[t+1];
            rgb_matrix[k++]=handle->static_mat_rgb[t+2];
            printf("%d rr %s\n",matrix[i][j],rgb_arr[matrix[i][j]]);
            //strcat(rgb_matrix,rgb_arr[matrix[i][j]]);
        }
}
void copy_matrix(int matrix[SNAPSHOT_HEIGHT][SNAPSHOT_WIDTH],void* data){
    int** data_matrix=(int**)(data);
    printf("%d",data_matrix);
    for(int i=0;i<SNAPSHOT_HEIGHT;i++){
        for(int j=0;j<SNAPSHOT_WIDTH;j++){
            matrix[i][j]=data_matrix[i][j];
            // printf("%d",data_matrix[i][j]);
        }
    }
}
void copy_rgb_matrix(int* rgb_matrix[SNAPSHOT_HEIGHT][SNAPSHOT_WIDTH],void* data){
    int*** data_matrix=(int***)(data);
    for(int i=0,j;i<SNAPSHOT_HEIGHT;i++){
        for(j=0;j<SNAPSHOT_WIDTH;j++){
            rgb_matrix[i][j]=data_matrix[i][j];
        }
    }
}

void free_matrix(int** m){
    for(int i=0;i<SNAPSHOT_HEIGHT;i++){
        free(m[i]);
        m[i]=NULL;
    }

}
void free_rgb_matrix(char * m){
    free(m);
}
size_t ppm_save(ppm_image *img) {
    printf("save ppm\n %d",img->width);
    FILE *   fp=fopen("img123.bmp","wb+");
    size_t n = 0;
    n +=fprintf(fp, "P6\n# THIS IS A COMMENT\n%d %d\n%d\n",
                img->width, img->height, 0xFF);
    n += fwrite(img->data, 1, img->width * img->height * 3, fp);
    fclose(fp);
    return n;
}
void save_snapshot(char* rgb_matrix){
    ppm_image p;
    p.height=SNAPSHOT_HEIGHT;
    p.width=SNAPSHOT_WIDTH;
    p.name="image.ppm";
    p.size=230400;
    p.data=rgb_matrix;


    // FILE *fp=NULL;
    ppm_save(&p);
}

void* capture(void* handle){
    printf("-----capture\n-----");
    handler * my_handler=(handler*)handle;
    stage my_stage=my_handler->stages[0];
    Node * n;
    my_stage.isActive=1;
    int i=0,** matrix=NULL;//[SNAPSHOT_HEIGHT][SNAPSHOT_WIDTH]={27};
    do{
        matrix=(int**)malloc(sizeof(int *)*SNAPSHOT_HEIGHT);
        for(int i=0;i<SNAPSHOT_HEIGHT;i++)  {
            matrix[i]=NULL;
            matrix[i]=(int *)malloc(sizeof(int)*SNAPSHOT_WIDTH);
        }
        randMat(matrix);
        n=createNode(matrix, sizeof(int));
        enqueu(my_stage.destQu,n);
        printf("capture %d\n",((int**)(n->data))[0][0]);
        sleep(3);
        i++;
    }while(my_handler->is_record_on);
}
void* rgb_converter(void* handle){
    printf("-----rgb_converter\n-----");
    handler * my_handler=(handler*)handle;
    stage my_stage=my_handler->stages[1];

    Node*n;
    my_stage.isActive=1;
    while(1){
        char *rgb_matrix;
        rgb_matrix=(char *)malloc(sizeof(char)*(SNAPSHOT_HEIGHT*SNAPSHOT_WIDTH*3));
        *rgb_matrix=0;
        n=dequeue(my_stage.sourseQu);
        //copy_matrix(matrix,(int **)n->data);
        covert_to_rgb(my_handler,rgb_matrix,(int**)n->data);
        free_matrix((int**)(n->data));
        freeNode(n);
        if(my_handler->is_snapshot_on)
        {
            ppm_image img;
            save_snapshot(rgb_matrix);
            my_handler->is_snapshot_on=0;
            free_rgb_matrix(rgb_matrix);
        }

        n=createNode(rgb_matrix,sizeof(rgb_matrix));
        printf("rgb \n");
        enqueu(my_stage.destQu,n);
        sleep(3);
    }
}
void* yuv_converter(void* handle){
    printf("-----yuv_converter\n-----");
    handler * my_handler=(handler*)handle;
    stage my_stage=my_handler->stages[2];
    Node*n;
    my_stage.isActive=1;
    //    int* rgb_matrix[SNAPSHOT_HEIGHT][SNAPSHOT_WIDTH];
    while(1){
        n=dequeue(my_stage.sourseQu);
        //       copy_rgb_matrix(rgb_matrix,n->data);
        printf("yuv \n");
        enqueu(my_stage.destQu,n);
        //  free(n);
        sleep(3);
    }
}
void* encode(void* handle){
    printf("-----cencode\n-----");

    handler * my_handler=(handler*)handle;
    stage my_stage=my_handler->stages[3];
    Node*n;
    my_stage.isActive=1;
    while(1){
        n=dequeue(my_stage.sourseQu);
        printf("encode\n");
        enqueu(my_stage.destQu,n);
        //   free(n);
        sleep(3);
    }
}

void* write_record(void*handle){
    printf("-----write\n-----");
    handler * my_handler=(handler*)handle;
    stage my_stage=my_handler->stages[4];
    Node*n;
    my_stage.isActive=1;

    while(1){
        n=dequeue(my_stage.sourseQu);
        printf("write \n");
        free_rgb_matrix((int***)(n->data));
        freeNode(n);
        sleep(3);
    }
}

void* GAS_API_init(){
    handler* handle=(handler*)malloc(sizeof(handler));
    if(handle==NULL){
        printf("error mallocing\n");
        exit(0);
    }
    //init queues
    int i=0;
    handle->stages[i].sourseQu=NULL;
    for(;i<STAGES_NUMBER-1;i++){
        handle->stages[i].destQu=createQueue(CAPACITY);
        handle->stages[i+1].sourseQu=handle->stages[i].destQu;
    }
    handle->stages[i].destQu=NULL;
    //init functions
    handle->stages[0].func=capture;
    handle->stages[1].func=rgb_converter;
    handle->stages[2].func=yuv_converter;
    handle->stages[3].func=encode;
    handle->stages[4].func=write_record;
    //init action
    handle->is_record_on=0;
    handle->is_snapshot_on=0;
    int j=0;
    char z=(char)255,x=0;
    while(j!=MAX_TEMP*3)
    {
        //rand numbers
        handle->static_mat_rgb[j++]=z;
        handle->static_mat_rgb[j++]=x;
        handle->static_mat_rgb[j++]=x;
    }
    handle->static_mat_rgb[9]=x;
    handle->static_mat_rgb[10]=x;
    handle->static_mat_rgb[11]=z;
    handle->static_mat_rgb[81]=x;
    handle->static_mat_rgb[82]=z;
    handle->static_mat_rgb[83]=x;
    return (void*)handle;
}
void GAS_API_free_all(void* handle){
    handler* my_handler=(handler*)(handle);

    for(int i=0;i<STAGES_NUMBER-1;i++){
        freeQueue(my_handler->stages[i].destQu);
        my_handler->stages[i].destQu=NULL;

    }
    free(my_handler);


}
int GAS_API_start_record(void* handle){
    printf ("====start_record====\n");
    handler* my_handler=(handler*)(handle);
    my_handler->is_record_on=1;
    for(int i=0;i<STAGES_NUMBER;i++){
        pthread_create(&(my_handler->stages[i].thread),NULL,(my_handler->stages)[i].func,my_handler);
    }
    return 0;

}
int GAS_API_stop_record(void* handle){

    printf ("====stop_record====\n\r");
    handler* my_handler=(handler*)(handle);
    for(int i=0;i<STAGES_NUMBER;i++){
        pthread_join(my_handler->stages[i].thread,NULL);
    }
    return 0;
}
int GAS_API_start_streaming(streaming_t* stream,char * file_name){
    printf("GAZ_API_start_streaming\n");
    return 1;
}
int GAS_API_stop_streamig(streaming_t*stream){
    printf("GAZ_API_stop_streamig");
    return 1;
}
int GAS_API_do_snapshot(handler* handle){
    printf("GAZ_API_stop_streamig");


    handler* my_handler=(handler*)(handle);
    my_handler->is_snapshot_on=1;
    if(my_handler->is_record_on)
    {
        return 1;}
    for(int i=0;i<2;i++){
        pthread_create(&(my_handler->stages[i].thread),NULL,(my_handler->stages)[i].func,my_handler);
    }
    for(int i=0;i<2;i++){
        pthread_join(my_handler->stages[i].thread,NULL);
    }

    return 1;
}
char* GAS_API_get_dll_version(){
    printf("GAZ_API_get_dll_version");
    return NULL;
}
char* GAS_API_get_video_statics(record_t* r){
    printf("GAZ_API_get_video_statics");
    return NULL;
}
char* GAS_API_get_status(){
    printf("GAZ_API_get_status");
    return NULL;
}

gazapi_t p_gaz_api= {
    .init=GAS_API_init,
    .free_all=GAS_API_free_all,
    .start_record=GAS_API_start_record,
    .stop_record=GAS_API_stop_record,
    .start_streaming=GAS_API_start_streaming,
    .stop_streamig=GAS_API_stop_streamig,
    .do_snapshot=GAS_API_do_snapshot,
    .get_dll_version=GAS_API_get_dll_version,
    .get_video_statics=GAS_API_get_video_statics,
    .get_status=GAS_API_get_status
};


