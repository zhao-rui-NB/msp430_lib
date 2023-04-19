#include <msp430g2553.h>

#include "uart.h"
UART uart;
   //uart  init
//uart.begin(dco_freq,baud_rate); //add after pin init
   //
//uart.have_data() : check can read data
//uart.read() : get a char from RX buffer
//uart.write(char); // write a char to TX buffer
//uart.print(int);
//uart.print(char*);// string address

int main(void){
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	uart.begin(1,9600);//(dco_freq , baud_rate)
	uart.print("UART begin\n");

	int data_cnt=0;
	while(1){ //data loop back
	    if(uart.have_data()){
	        char ch = uart.read();
	        uart.write(ch);
	        if(ch=='\n'){
	            data_cnt++;
	            uart.print("data cnt = ");
                uart.print(data_cnt);
                uart.print(" , 我收到該死的資料了:");
	        }
	    }
	}




	return 0;
}
