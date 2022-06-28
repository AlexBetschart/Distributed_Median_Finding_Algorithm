//@author: Alex Betschart
//Distributed Median Finding Algorithm using pipes
//June 24 2022

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#define REQUEST 100
#define PIVOT 200
#define LARGE 300
#define SMALL 400
#define READY 500
#define STOP 600
#define NUMCHILD 5

int main(){
  int my_num = 1;
  int i;
  
  //Defining codes
  int request = REQUEST;
  int pivot = PIVOT;
  int large = LARGE;
  int small = SMALL;
  int ready = READY;
  int stop = STOP;
  srand(time(0));

  //Create 10 pipes, 5 for each child to parent
  // 5 for parent to each child.
  printf("Creating pipes.\n");
  int fds[NUMCHILD*2][2];
  for (int i = 0; i < NUMCHILD*2; i++){
    pipe(fds[i]);
  }
  printf("pipes created.\n");

  //Child processes
  //pipes in use: 
  //write to parent: fds[i][1]
  //read from parent: fds[i+5][0]
  for (i = 0; i < 5; i++){
    fflush(stdout);
    printf("Creating child %d...\n", i);
    if (fork()==0){
      
      printf("%d waiting for id from parent\n", i);
      int id;
      int read_pipe = i+5;
      int write_pipe = i;
      int parent_message;
      int nums[5];
      int array_is_empty = -1;
      int pivot_num;
      int greater_than_pivot = 0; 

      //get id from parent
      read(fds[read_pipe][0], &parent_message, sizeof(int));
      id = parent_message;
      printf("%d received id from parent: %d\n",i,id);
      printf("Reading from file\n");
      char file_name[50];
      char base[] = "input_\0";
      char ending[] = ".txt\0";
      char number[2];
      sprintf(number,"%d",id);

      strncat(base,number,strlen(number));
      strncat(base, ending,strlen(ending));
      strcpy(file_name, base);
      puts(file_name);
      //sleep(1);
      int i;
      int count = 0;
      FILE *fp;
      fp = fopen(file_name,"r");
      while (!feof (fp)){
        fscanf(fp,"%d",&i);
        nums[count] = i;
        count++;
      }
      fclose(fp);
      printf("%d reading from file: ",id);
      for (int i = 0; i < 5; i++){
        printf("%d ", nums[i]);
      }
      printf("\n\n");
      //sleep(3);
      int empty = 0;
      //Send ready code
      printf("%d Sending ready code to parent...\n", id);
      write(fds[write_pipe][1], &ready, sizeof(int));

      printf("%d Entering while loop...\n\n", id);
      while(parent_message != stop){
        read(fds[read_pipe][0], &parent_message, sizeof(int));
        if (parent_message == request){
          printf("%d received REQUEST\n", id);
          int empty = 1;
          for(int i = 0; i < 5; i++){
            if (nums[i] != 0){
              empty = 0;
            }
          }
          if (empty == 0){
            int selected_zero = 1;
            int random = rand() % 5;
            while(selected_zero==1){
              if (nums[random] != 0){
                selected_zero = 0;
              } else {
                random = rand() % 5;
              }
            }
            printf("%d selected %d to send to parent\n",id,nums[random]);
            //sleep(2);
            write(fds[write_pipe][1], &nums[random], sizeof(int));
          } else {
            printf("%d array is empty. Sending -1\n",id);
            write(fds[write_pipe][1], &array_is_empty, sizeof(int));
          }
        } else if ( parent_message == pivot){
          printf("%d received PIVOT\n", id);
          read(fds[read_pipe][0], &pivot_num, sizeof(int));
          greater_than_pivot = 0;
          for (int i = 0; i < 5; i++){
            if (nums[i] > pivot_num){
              greater_than_pivot++;
            }
          }
          printf("%d has %d elements greater than the pivot %d\n",id,greater_than_pivot,pivot_num);
          write(fds[write_pipe][1], &greater_than_pivot, sizeof(int));
        } else if ( parent_message == small) {
          printf("%d received SMALL\n", id);
          for (int i = 0; i < 5; i++){

            if (nums[i] < pivot_num){
              nums[i] = 0;
            }
          }
        } else if (parent_message == large){
          printf("%d received LARGE\n",id);
          for (int i = 0; i < 5; i++){
            if (nums[i] > pivot_num){
              nums[i] = 0;
            }
          }
        }
      
      }

      //end
      exit(0);
    }
  }
  //parent process
  //read from child i: read(fds[i][0], &messageholder, sizeof(int))
  //write to child i: write(fds[i+5][1], &message, sizeof(int))
  printf("Parent entered\n");
 
  //send ids to children
  int to_child[5];
  int from_child[5];
  int parent_pivot;
  int k = 25/2;
  printf("Parent sending children ids...\n");
  for (int i = 0; i < 5; i++){
    to_child[i] = i+1;
    write(fds[i+5][1], &to_child[i], sizeof(int));
  }
  //wait for ready codes
  for (int i = 0; i < 5; i++){
    read(fds[i][0], &from_child[i], sizeof(int));
  }
  for (int i = 0; i < 5; i++){
    if (from_child[i] != ready){
      printf("Something broke. Ready code not received.\n");
    }
  }
  printf("Parent received ready codes\n");
  //sleep(1);

  //send random child request code
  //If child returns -1, ask another child.
  int rand_child;
  int must_request = 0;

  while(1){
    srand(rand());
    rand_child = rand() % 5;
    printf("rand_child is: %d\n",rand_child);
    printf("First while loop\n");
    printf("k = %d\n", k);
    must_request = 0;
    while (must_request == 0){
      printf("Second while loop\n");
      from_child[rand_child] = -1;
      write(fds[rand_child+5][1], &request, sizeof(int));
      printf("Parent sent REQUEST to %d\n",rand_child+1);
      read(fds[rand_child][0], &from_child[rand_child],sizeof(int));
      printf("Parent received %d from %d\n",from_child[rand_child],rand_child+1);
      //sleep(2);
      if (from_child[rand_child] != -1){
        must_request = 1;
      } else {
        rand_child = rand() % 5;
      }
    }
    printf("First while loop again\n");
    parent_pivot = from_child[rand_child];

    //broadcast pivot to children
    for (int i = 0; i < 5; i++){
      write(fds[i+5][1], &pivot, sizeof(int));
      write(fds[i+5][1], &parent_pivot, sizeof(int));
    }

    int sum_total = 0;
    for (int i = 0; i < 5; i++){
      read(fds[i][0], &from_child[i], sizeof(int));
    }

    for (int i = 0; i < 5; i++){
      sum_total = sum_total + from_child[i];
    }

    if (sum_total == k){
      printf("%d is the median!\n", parent_pivot);
      for (int i = 0; i < 5; i++){
        int stop = STOP;
        write(fds[i+5][1], &stop, sizeof(int));
      }
      exit(0);
    } else if (sum_total > k){
      for (int i = 0; i < 5; i++){
        write(fds[i+5][1], &small, sizeof(int));
      }
    } else if (sum_total < k){
      for (int i = 0; i < 5; i++){
        write(fds[i+5][1], &large, sizeof(int));
      }
      k = k - sum_total;
    }
  }

  exit(0);
}

//create n*
// void create_pipes(){

// }

//get filename in the form "input_i.txt" where i is the number of the file.
// char* get_file_contents(int num){
//   char file_name[12];
//   char base[] = "input_";
//   char ending[] = ".txt";
//   char* number[1];
//   sprintf(number,"%d",num);

//   strncat(file_name, base,strlen(base));
//   strncat(file_name, (char*)number,strlen(number));
//   strncat(file_name, ending,strlen(ending));
//   puts(file_name);
//   return file_name;
// }

// void received_code(int *int_array, int length){
//   for (int i=0; i <10;i++){
    
//   }
// }

// void close_unused_pipes(int an_id){
//   for (int i = 0; i < NUMCHILD; i++){
//     if (i != an_id + 5 && i != an_id){
//       close()
//     } 
//   }
// }