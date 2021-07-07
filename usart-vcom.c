#include "stm32l476xx.h"

void RCC_Init(void) {
	
	// Enable the clock of GPIO
	RCC->AHB2ENR |= RCC_AHB2ENR_GPIODEN;
	
	// Enable the clock of USART 2
	RCC->APB1ENR1  |= RCC_APB1ENR1_USART2EN;            // Enable USART 2 clock		
		
}

void GPIO_Init()
{
	// ********************** USART 2 ***************************
	// USART2_TX:  PD.5          USART2_RX:  PD.6
	// **********************************************************

	GPIOD->MODER  	&= ~(0x0F << (2*5));   // Clear bits for PD5,PD6
	GPIOD->MODER  	|= 0x0A << (2*5);      // Input(00, reset), Output(01), AlterFunc(10), Analog(11)

	// GPIOD->AFR[0] for PIN.0 - PIN.7
	// GPIOD->AFR[1] for PIN.8 - PIN.15
	GPIOD->AFR[0] 	|= 0x77 << (4*5); //AF7(USART1..3)
	GPIOD->OSPEEDR  |=   0x0F<<(2*5); // 400 KHz(00), 2 MHz(01), 10 MHz(01), 40 MHz (11)
	GPIOD->OTYPER   &=  ~(0x1<<5) ; // TX pin (PA.2) should be set up as a push-pull output
	GPIOD->OTYPER   |=   (0x1<<6) ;   // RX pin (PA.10) is a floating input	
	GPIOD->PUPDR    &= ~(0x0F<<(2*5)) ; //No pull-up/pull-down
} 

void USART_Init (USART_TypeDef * USARTx) {
	// Default setting: No hardware flow control, 8 data bits, no parity, and one stop bit		
	 //USARTx->BRR   = 8000000/9600;					 						// BRR = System Frequency/BAUDRATE
	
	
	/* Configure word length to 8 bit. */
	USARTx->CR1 &= ~USART_CR1_M;
	
	/*Configure oversampling to x16 */
	USARTx->CR1 &= ~USART_CR1_OVER8;
	
	/* Configure stop bits to 1 stop bit. */
	USARTx->CR2 &= ~USART_CR2_STOP;
	
	/* Configure baud rate register for 9600 bps. */
	USARTx->BRR = 8000000/9600;

	USARTx->CR1  |= USART_CR1_UE;  
	
	USARTx->CR1  |= (USART_CR1_RE | USART_CR1_TE);  			// Transmitter and Receiver enable
	                  			// USART enable
}


uint8_t USART_Read (USART_TypeDef * USARTx)  {
	// SR_RXNE (Read data register not empty) bit is set by hardware
	while (!(USARTx->ISR & USART_ISR_RXNE));  						// Wait until RXNE (RX not empty) bit is set
	// USART resets the RXNE flag automatically after reading DR
	return ((uint8_t)(USARTx->RDR & 0x1FF));
	// Reading USART_RDR automatically clears the RXNE flag 
}


void USART_Write(USART_TypeDef * USARTx, uint8_t ch) {
	USARTx->TDR = (ch & 0x1FF);
	while (!(USARTx->ISR & USART_ISR_TXE));   							// wait until TXE (TX empty) bit is set 
}   
 


void System_Clock_Init(void){
	
	RCC->CR |= RCC_CR_MSION; 
	
	// Select MSI as the clock source of System Clock
	RCC->CFGR &= ~RCC_CFGR_SW; 
	
	// Wait until MSI is ready
	while ((RCC->CR & RCC_CR_MSIRDY) == 0); 	
	
	// MSIRANGE can be modified when MSI is OFF (MSION=0) or when MSI is ready (MSIRDY=1). 
	RCC->CR &= ~RCC_CR_MSIRANGE; 
	RCC->CR |= RCC_CR_MSIRANGE_7;  // Select MSI 8 MHz	
 
	// The MSIRGSEL bit in RCC-CR select which MSIRANGE is used. 
	// If MSIRGSEL is 0, the MSIRANGE in RCC_CSR is used to select the MSI clock range.  (This is the default)
	// If MSIRGSEL is 1, the MSIRANGE in RCC_CR is used. 
	RCC->CR |= RCC_CR_MSIRGSEL; 
	
	// Enable MSI and wait until it's ready	
	while ((RCC->CR & RCC_CR_MSIRDY) == 0); 		
}


#define BufferSize 32
 

uint8_t USART1_Buffer_Rx[BufferSize] = {0xFF};

uint8_t Tx_Counter = 0;
uint8_t Rx_Counter = 0;
	 

int main(void){
	
	System_Clock_Init(); // Set System Clock as 8 MHz
	RCC_Init();
	GPIO_Init();
	USART_Init(USART2);

	while(1) {  
		USART1_Buffer_Rx[Tx_Counter] = USART_Read(USART2);   
		USART_Write(USART2, USART1_Buffer_Rx[Tx_Counter]);    //
		Tx_Counter++;
		if (Tx_Counter==BufferSize) 
			Tx_Counter = 0;
	}  
 	
}
