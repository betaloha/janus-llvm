#include <stdio.h>

int foo(){
int j=1, k=2;
fprintf(stderr, "foooooooo\n");
return j+k;
}

int bar(int z){
fprintf(stderr, "baaaaaar\n");
return z+1142;
}

int laa(){
fprintf(stderr, "laaaaaaaaaa\n");
return 1;
}

int main(){
foo();
bar(1241);

}
