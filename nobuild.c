#include <ctype.h>

#define NOBUILD_IMPLEMENTATION
#include "nobuild.h"

#define CFLAGS "-Wall", "-Wextra", "-std=c99", "-pedantic"

#include <stdint.h>
#define HEAP_SIZE 1024 * 1024 * 1024
static uint8_t *heap = NULL;
static size_t heap_top = 0;

void allocator_init(void){
    heap = (uint8_t *)malloc(HEAP_SIZE);
    memset(heap,0,HEAP_SIZE);
	assert(heap != NULL);
}

static void *allocate(size_t size) {
    if(heap == NULL){
        allocator_init();
    }
	size_t old_top = heap_top;
	heap_top += size;
	assert(heap_top <= HEAP_SIZE);
	return &heap[old_top];
}

void foreach_file_in_dir(const char *dirpath,void (*func)(const char*,const char*,void*),void* udata) {
    do {                                                
        struct dirent *dp = NULL;                       
        DIR *dir = opendir(dirpath);                    
        if (dir == NULL) {                              
            PANIC("could not open directory %s: %s",    
                  dirpath, nobuild__strerror(errno));   
        }                                               
        errno = 0;                                      
        while ((dp = readdir(dir))) {                   
            const char *file = dp->d_name;
            func(dirpath,file,udata);            
          
        }                                                                                      
                                                        
        if (errno > 0) {                                
            PANIC("could not read directory %s: %s",    
                  dirpath, nobuild__strerror(errno));   
        }                                               
                                                        
        closedir(dir);                                  
    } while(0);
}

typedef struct {
    Cstr_Array deps;
    Cstr_Array filewaiting;
    Fd to_write;
} write_data_t;
int has_dep(char* buffer, size_t size){
    char* dep = strstr(buffer,"#include \"nobuild_");
    return dep != NULL;
}

char* remove_deps(char* buffer, size_t size){

    size_t bytes = size;
    Cstr_Array deps = {0};
    char* dep = strstr(buffer,"#include \"nobuild_");
    char* dep2 = NULL;
    if(dep != NULL)
        dep2 = strstr(dep,"\"\n");
    while(dep != NULL && dep2 != NULL){
        char* out_name = allocate(sizeof(char) * 260);
        size_t out_len = dep2 - dep + 1;
        memcpy(out_name,dep,out_len);
        if(deps.count == 0){
            deps = CSTR_ARRAY_MAKE(out_name);
        }
        else {
            deps = cstr_array_append(deps,out_name);
        }
        int c = 0;
        while(c != out_len+1){
            dep[c++] = '/';
        }
        size_t move = dep2 - (char*)buffer;
        dep = strstr(buffer+move,"#include \"nobuild_");
        if(dep != NULL)
            dep2 = strstr(dep,"\"\n");
        else {
            buffer[move+1] = '\n';
        }
    }
    char* out = allocate(strlen(buffer));
    strcpy(out,buffer);

    return out;
}

void write_h(const char* dirpath, const char* filename, void* udata){
    write_data_t* data = (write_data_t*)udata;

    for(int i = 0; i < data->deps.count;++i){
        char* dep = data->deps.elems[i];
        
        if(strcmp(filename,dep) == 0){
            unsigned char buffer[4096];
            char* path = allocate(sizeof(char) * 260);
            snprintf(path,260,"%s%s%s",dirpath,PATH_SEP,filename);
            Fd r_h = fd_open_for_read(path);
            size_t bytes = fd_read(r_h,buffer,4096);
            if(has_dep(buffer,bytes) && cstr_ends_with(filename,".h")){
                if(data->filewaiting.count == 0){
                    data->filewaiting = CSTR_ARRAY_MAKE(path);
                }
                else{
                    data->filewaiting = cstr_array_append(data->filewaiting,path);
                }
            }
            else{
                char* out = remove_deps(buffer,bytes);
                fd_write(data->to_write,out,bytes);
                while(1){
                    bytes = fd_read(r_h,buffer,4096);
                    if(bytes == 0){
                        fd_write(data->to_write,"\n",1);
                        break;
                    }
                    fd_write(data->to_write,buffer,bytes);
                }
            }
            fd_close(r_h);
            break;
        }
    }

}

int main(int argc, char **argv)
{
    // GO_REBUILD_URSELF(argc, argv);

    // Build standalone versions of each nobuild header
    if (!path_exists("generate")) {
        MKDIRS("generate");
    }

    Fd nbh = fd_open_for_read("src/nobuild.h");
    Fd nbsh = fd_open_for_write("generate/nobuild.h");


    unsigned char buffer[4096] = {0};
    size_t bytes = fd_read(nbh,buffer,sizeof(buffer));
    char* minirent = allocate(strlen("#include \"minirent.h\""));
    strcpy(minirent,"#include \"minirent.h\"");
    Cstr_Array deps = CSTR_ARRAY_MAKE(minirent);
    char* dep = strstr(buffer,"#include \"nobuild_");
    char* dep2 = strstr(dep,"\"\n");
    while(dep != NULL && dep2 != NULL){
        char* out_name = allocate(sizeof(char) * 260);
        size_t out_len = dep2 - dep + 1;
        memcpy(out_name,dep,out_len);
        deps = cstr_array_append(deps,out_name);
        
        int c = 0;
        while(c != out_len+1){
            dep[c++] = '/';
        }
        size_t move = dep2 - (char*)buffer;
        dep = strstr(buffer+move,"#include \"nobuild_");
        if(dep != NULL)
            dep2 = strstr(dep,"\"\n");
        else {
            buffer[move+1] = '\n';
        }
    }
    char* nobuild_h = allocate(strlen(buffer));
    strcpy(nobuild_h,buffer);

    for (size_t elem_index = 0; elem_index < deps.count; ++elem_index){
        char *dep = deps.elems[elem_index];
        int c = 0;
        while(dep[c++] != '\"'){}
        int c2 = 1;
        while(dep[c+c2++] != '\"'){}
        c2--;
        char temp[260]={0};
        int c3 =0;
        while(c3 < c2){
            temp[c3++] = dep[c+c3];
        }
        strcpy(dep,temp);
    }
    write_data_t w_data = {.deps = deps,.to_write=nbsh,.filewaiting={0}};
    foreach_file_in_dir("src",write_h,&w_data);
    char cjson_path[260] = {0};
    snprintf(cjson_path,260,"src%scJSON.h",PATH_SEP);
    w_data.filewaiting = cstr_array_append(w_data.filewaiting,cjson_path);
    for(int i = 0; i < w_data.filewaiting.count;++i){
        char* path = w_data.filewaiting.elems[i];
        Fd fd = fd_open_for_read(path);
        unsigned char buffer[4096] = {0};
        size_t bytes = fd_read(fd,buffer,4096);
        char* out = remove_deps(buffer,bytes);
        fd_write(nbsh,out,strlen(out));
        while(1){
            bytes = fd_read(fd,buffer,4096);
            if(bytes == 0){
                fd_write(nbsh,"\n",1);
                break;
            }
            fd_write(nbsh,buffer,bytes);
        }
    }

    // Write the nobuild.h header
    fd_write(nbsh,nobuild_h,strlen(nobuild_h));

    //Start writing the implementation
    const char* def = 
    "\n////////////////////////////////////////////////////////////////////////////////\n"
    "#ifdef NOBUILD_IMPLEMENTATION\n\n"
    "////////////////////////////////////////////////////////////////////////////////\n";
    fd_write(nbsh,def,strlen(def));
    memset(&w_data.filewaiting,0,sizeof(w_data.filewaiting));
    for(int i= 0; i < deps.count;++i){
        char* name = deps.elems[i];
        name[strlen(name)-1] = 'c';
    }
    foreach_file_in_dir("src",write_h,&w_data);

    // Write cJSON.c file
    cjson_path[strlen(cjson_path)-1] = 'c';
    Fd cjson_c = fd_open_for_read(cjson_path);
    memset(buffer,0,4096);
    while(1){
        bytes = fd_read(cjson_c,buffer,4096);
        if(bytes == 0){
            fd_write(nbsh,"\n",1);
            break;
        }
        fd_write(nbsh,buffer,bytes);
    }

    const char* enddef = 
    "////////////////////////////////////////////////////////////////////////////////\n"
    "#endif //NOBUILD_IMPLEMENTATION\n\n"
    "////////////////////////////////////////////////////////////////////////////////\n";

    fd_write(nbsh,enddef,strlen(enddef));

    fd_close(nbsh);
    Cstr_Array output = cstr_array_make("");
    // int i = 0;
    // while(1){
        
    //     if(bytes == 0){
    //         break;
    //     }

    //     if(i == 0){
            
    //     }

    //     fd_write(ndsh,)
    //     i++;
    // }

    return 0;
}
