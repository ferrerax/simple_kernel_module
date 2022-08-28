#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define BUFFER_IN_SIZE	10
#define BUFFER_OUT_SIZE 20	


int main(){

    char buffer_in[BUFFER_IN_SIZE];
    char buffer_out[BUFFER_OUT_SIZE];
    char * str = "En pinxo li va dir a en panxo vols que et punxi amb un punxo i el panxo li va dir a en pinxo punxam pero a la panxa no. El gegant del pi ara balla ara balla, el gegant del pi ara balla pel cami. El gegant de la ciutat ara balla ara balla el gegant de la ciutat ara balla pel terrat";
    int f;

    if ((f = open("/proc/quim_module", O_RDWR)) < 0){
	printf("Error opening proc file");
	return -1;
    }


    for (int i = 0; i < strlen(str); i+=BUFFER_IN_SIZE){
	    memcpy(buffer_in,str+i,BUFFER_IN_SIZE);
	    write(f,buffer_in,BUFFER_IN_SIZE);
    }


    printf("Buffer ple, procedim a buidar\n");

    int r = read(f, buffer_out, BUFFER_OUT_SIZE);
    while (r) {
	    for(int i = 0; i < BUFFER_OUT_SIZE; i++){
		    printf("%c", buffer_out[i]);
	    }
	    r = read(f, buffer_out, BUFFER_OUT_SIZE) != 0;
    }

}
