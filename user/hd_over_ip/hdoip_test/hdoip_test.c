
// File: hpoid_test.c

#include <stdio.h>
#include "hdt_ether.h"
#include "hdt_video.h"


int do_it (char choice)
{
   int retval = 0;
   switch (choice)
   {
      case 'a': case 'A':
			printf ("\n  selected:  Ethernet Connection Test\n");  
			ethernet_test();           
		break;        
		
      case 'b': case 'B':
			printf ("\n  selected:  Video Tests\n"); 
			video_test();         
		break;          
         
      case 'c': case 'C':
 			printf ("\n  selected:  USB Tests\n");   
		break;          

      case 'q': case 'Q':
		retval = 1;
		break; 
		         
      default:
         printf ("\n  ERROR: Wrong input! Try again.");
         retval = 1;
         break;
   }
   return retval;
} 

void print_menu () //Prints menu on screen
{
	int i = 0;
	for(i=0;i<10;i++) printf("\n");
	//printf("\033[2J"); //Clears system screen	
	printf("\n\n----------------------------------------------------");	
	printf("\n\n    H D   o v e r   I P       S Y S T E M T E S T ");
	printf("\n\n----------------------------------------------------");
	printf ("\n\nChoose one of the following:\n");
	printf ("\n  A:  Ethernet Connection Test\n");	
	printf ("\n  B:  Video Tests\n");	
	printf ("\n  C:  USB Tests\n");
	printf ("\n  Q:  Quit.");
	printf ("\n\n  Enter a choice in { A, B, C, Q } : ");
}

void menu ()
{
   int done = 0;
   char choice;
   while (!done)
   {
      print_menu ();
      scanf (" %c", &choice);
      printf("\n\n");
      done = do_it (choice);  //Call function do_it
   }
}

int main(int argc, char *argv[])
{
	menu();			
	return 0;
}
