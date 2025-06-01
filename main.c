#include <LPC21xx.H>

// LCD pin definitions
#define LCD_RS    (1<<8)
#define LCD_E     (1<<9)
#define LCD_DATA  (0xF<<10)  // D4-D7 on P0.10-P0.13

// Function prototypes
void delay_ms(unsigned int ms);
void LCD_Init(void);
void LCD_Cmd(unsigned char cmd);
void LCD_Data(unsigned char data);
void LCD_String(char *str);
void ADC_Init(void);
unsigned int ADC_Read(unsigned char channel);
float Read_Temperature(void);

int main(void)
{
    float temperature;
    char temp_str[16];
    
    // Initialize peripherals
    LCD_Init();
    ADC_Init();
    
    // Display startup message
    LCD_Cmd(0x80);  // Move cursor to beginning of first line
    LCD_String("Temp Monitor");
    delay_ms(1000);
    
    while(1)
    {
        temperature = Read_Temperature();
        
        // Display temperature in Celsius
        LCD_Cmd(0x80);  // First line
        LCD_String("Temp: ");
        sprintf(temp_str, "%.2f C", temperature);
        LCD_String(temp_str);
        
        // Display temperature in Fahrenheit
        LCD_Cmd(0xC0);  // Second line
        LCD_String("      ");
        sprintf(temp_str, "%.2f F", (temperature * 9.0/5.0) + 32.0);
        LCD_String(temp_str);
        
        delay_ms(1000);  // Update every second
    }
}

// Delay function
void delay_ms(unsigned int ms)
{
    unsigned int i, j;
    for(i=0; i<ms; i++)
        for(j=0; j<2000; j++);
}

// LCD Initialization
void LCD_Init(void)
{
    IODIR0 |= LCD_RS | LCD_E | LCD_DATA;  // Set pins as output
    
    delay_ms(20);  // LCD power-up delay
    
    // Initialization sequence for 4-bit mode
    LCD_Cmd(0x33);
    LCD_Cmd(0x32);
    LCD_Cmd(0x28);  // 4-bit, 2-line, 5x8 font
    LCD_Cmd(0x0C);  // Display on, cursor off
    LCD_Cmd(0x06);  // Increment cursor
    LCD_Cmd(0x01);  // Clear display
    delay_ms(2);
}

// Send command to LCD
void LCD_Cmd(unsigned char cmd)
{
    IOCLR0 = LCD_RS;  // RS=0 for command
    
    // Send high nibble
    IOSET0 = (cmd & 0xF0) << 6;  // Map to P0.10-P0.13
    IOSET0 = LCD_E;  // Enable high
    delay_ms(1);
    IOCLR0 = LCD_E;  // Enable low
    
    // Send low nibble
    IOSET0 = (cmd & 0x0F) << 10;  // Map to P0.10-P0.13
    IOSET0 = LCD_E;  // Enable high
    delay_ms(1);
    IOCLR0 = LCD_E;  // Enable low
    
    delay_ms(2);  // Command execution time
}

// Send data to LCD
void LCD_Data(unsigned char data)
{
    IOSET0 = LCD_RS;  // RS=1 for data
    
    // Send high nibble
    IOSET0 = (data & 0xF0) << 6;  // Map to P0.10-P0.13
    IOSET0 = LCD_E;  // Enable high
    delay_ms(1);
    IOCLR0 = LCD_E;  // Enable low
    
    // Send low nibble
    IOSET0 = (data & 0x0F) << 10;  // Map to P0.10-P0.13
    IOSET0 = LCD_E;  // Enable high
    delay_ms(1);
    IOCLR0 = LCD_E;  // Enable low
    
    delay_ms(1);  // Data execution time
}

// Display string on LCD
void LCD_String(char *str)
{
    while(*str)
    {
        LCD_Data(*str++);
    }
}

// ADC Initialization
void ADC_Init(void)
{
    PINSEL1 |= (1<<24);  // Set P0.28 as AD0.0
    ADCR = (1<<0) |      // Select AD0.0
           (4<<8) |      // CLKDIV = 4 (ADC clock = 3MHz if PCLK=15MHz)
           (1<<21);       // Enable ADC
}

// Read ADC channel
unsigned int ADC_Read(unsigned char channel)
{
    ADCR &= ~0xFF;  // Clear channel selection
    ADCR |= (1<<channel);  // Select channel
    
    ADCR |= (1<<24);  // Start conversion
    while(!(ADDR & (1<<31)));  // Wait for conversion complete
    
    ADCR &= ~(1<<24);  // Stop conversion
    
    return ((ADDR >> 6) & 0x3FF);  // Return 10-bit result
}

// Read temperature in Celsius
float Read_Temperature(void)
{
    unsigned int adc_value = ADC_Read(0);  // Read from AD0.0
    float voltage = (adc_value * 3.3) / 1023.0;  // Convert to voltage (3.3V reference)
    return voltage * 100.0;  // LM35 outputs 10mV per °C
}
