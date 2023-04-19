#ifndef uart_h
    #define uart_h

#include <msp430g2553.h>

#define BUF_SIZE 128

//fifo for RX and TX
class FIFO{
    volatile char buf[BUF_SIZE];
    volatile int front = 0;
    volatile int back = 0;
    volatile int data_cnt = 0;

    public:
        void push(char data){
            if(data_cnt<BUF_SIZE){
                buf[back] = data;
                back++;
                back %= BUF_SIZE;
                data_cnt++;
            }
        }
        char pop(){
            char r = -1;
            if(data_cnt>0){
                r = buf[front];
                front++;
                front %= BUF_SIZE;
                data_cnt--;
            }
            return r;
        }
        char IS_full(){
            return data_cnt>=BUF_SIZE;
        }
        char IS_empty(){
            return data_cnt==0;
        }
};



class UART{

    public:
        FIFO TX_fifo;
        FIFO RX_fifo;

        void begin(int smclk_MHZ , unsigned long baudrate){

            switch (smclk_MHZ){
                case 16: BCSCTL1 = CALBC1_16MHZ; DCOCTL = CALDCO_16MHZ; break;
                case 12: BCSCTL1 = CALBC1_12MHZ; DCOCTL = CALDCO_12MHZ; break;
                case  8: BCSCTL1 = CALBC1_8MHZ; DCOCTL = CALDCO_8MHZ; break;
                case  1: BCSCTL1 = CALBC1_1MHZ; DCOCTL = CALDCO_1MHZ; break;
                default: BCSCTL1 = CALBC1_1MHZ; DCOCTL = CALDCO_1MHZ; break;
            }
            unsigned long n = (smclk_MHZ*1000000.0/baudrate);
            UCA0BR0 = n&0xFF ;             
            UCA0BR1 = n>>8 ;
            unsigned long p = (smclk_MHZ*1000000.0/baudrate - n)*8;
            UCA0MCTL = p<<1;

            UCA0CTL1 |= UCSSEL_2; // SMCLK
            BCSCTL2 &= ~SELS;     // smclk form DCOCLK

            P1SEL = (BIT1 + BIT2);      P1SEL2 = (BIT1 + BIT2);

            UCA0CTL1 &= ~(UCSWRST);
            IE2 |= UCA0RXIE;
            _bis_SR_register(GIE);
        }

        char have_data(){
            return !RX_fifo.IS_empty();
        }

        char read(){
            if(RX_fifo.IS_empty()){
                return -1;
            }
            else{
                return RX_fifo.pop();
            }
        }
        void write(char data){
            while(TX_fifo.IS_full());
            TX_fifo.push(data);
            IE2 |= UCA0TXIE;
        }

        void print(){

            for(char i = 'A' ; i<='Z' ; i++){
                TX_fifo.push(i);
            }
            IE2 |= UCA0TXIE;
            TX_fifo.push('\r');
            TX_fifo.push('\n');
        }

        void print(char* str){
            for(; *str != '\0' ; str++){
                write(*str);
            }
            IE2 |= UCA0TXIE;
        }
        void print(int num){
            char num_bufer[11];
            num_bufer[10] = '\0';
            char sign = num<0;//+:0 , -:1
            char *p = num_bufer+9;
            num = (sign)?(num*-1):num;

            if(num==0){
                *p = '0';
                p--;
            }

            while(num){
                *p = '0'+num%10;
                num /= 10;
                p--;
            }
            if(sign){
                *p = '-';
            }
            else{
                p++;
            }
            print(p);
        }
};

/////////////////////////////////////////////////////////////////////

extern UART uart;

#pragma vector = USCIAB0TX_VECTOR
__interrupt void  USCI0TX_ISR(void){
    if(uart.TX_fifo.IS_empty()){
        IE2 &= ~(UCA0TXIE);
    }
    else{
        UCA0TXBUF = uart.TX_fifo.pop();
        _bis_SR_register(GIE);
    }
}//end of USCI0TX_ISR()

#pragma vector = USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void){
    if(uart.RX_fifo.IS_full()){

    }
    else{
        uart.RX_fifo.push(UCA0RXBUF);
        _bic_SR_register_on_exit(LPM4_bits);
    }
}//end of USCI0RX_ISR()


#endif
