#include <xc.h>
#define _XTAL_FREQ 4000000

#pragma config FOSC = HS, WDTE = OFF, PWRTE = OFF, BOREN = ON, LVP = OFF, CPD = OFF, WRT = OFF, CP = OFF

void ADC_Init(void)
{
    ADCON0 = 0x41;  // Channel 0, ADC ON
    ADCON1 = 0x80;  // Right justified, AN0 analog
}

unsigned int ADC_Read(void)
{
    __delay_ms(2);
    GO_nDONE = 1;
    while(GO_nDONE);
    return ((ADRESH<<8) + ADRESL);
}

void SPI_SlaveInit(void)
{
    TRISA5 = 1;    // SS as input
    TRISC3 = 1;    // SCK input
    TRISC4 = 1;    // SDI input
    TRISC5 = 0;    // SDO output

    SSPSTAT = 0x40;
    SSPCON = 0x24; // SPI Slave mode, SS enabled
}

void UART_Init(void)
{
    SPBRG = 25;    // 9600 baud @ 4MHz, BRGH=1
    TXSTA = 0x24;
    RCSTA = 0x90;
}

void UART_Tx(char c)
{
    while(!TXIF);
    TXREG = c;
}

void UART_TxString(const char *str)
{
    while(*str) UART_Tx(*str++);
}

void main(void)
{
    unsigned int adc_value;
    unsigned char temperature;

    ADC_Init();
    UART_Init();
    SPI_SlaveInit();

    UART_TxString("Slave Ready\r\n");

    while(1)
    {
        // Wait until SS is pulled low by master
        while(RA5);  // SS high ? wait

        // Read ADC and convert
        adc_value = ADC_Read();
        temperature = adc_value * 0.488;

        // Load new data for next SPI clock
        SSPBUF = temperature;

        while(!BF); // Wait for transfer complete

        UART_TxString("Temp Sent: ");
        UART_Tx((temperature/10) + '0');
        UART_Tx((temperature%10) + '0');
        UART_TxString(" C\r\n");
        __delay_ms(1000);
    }
}
