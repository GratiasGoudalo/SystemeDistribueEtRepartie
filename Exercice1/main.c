#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>

//Fonction affichant le numéro du fils et le PID
void fils (int i)
{
    printf ("Processus numero: %i Le PID est : %i \n", i, getpid());
    exit(0);
}

int main ()
{

int i;
//Boucle de creation et d'affichage des PID des trois fils.
for (i=1; i<4; i++) {
if(fork()== 0) {
printf ("creation du Processus: ", i);
fils(i);
exit(0);
}
}
sleep(2);
//printf ("Voila un dernier message \n" );
return (0);
}

