/**
  Generated Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This is the main file generated using PIC10 / PIC12 / PIC16 / PIC18 MCUs

  Description:
    This header file provides implementations for driver APIs for all modules selected in the GUI.
    Generation Information :
        Product Revision  :  PIC10 / PIC12 / PIC16 / PIC18 MCUs - 1.81.7
        Device            :  PIC12F1840
        Driver Version    :  2.00
*/


#include "mcc_generated_files/mcc.h"

#define LED1        LATAbits.LATA1  // RA0 is LED for debugging purposes
#define LED2        LATAbits.LATA5  // RA1 is second debug LED
#define BUZZER      LATAbits.LATA2
#define LEDON  0
#define LEDOFF 1
#define BUZZON 0
#define BUZZOFF 1
#define BUZZERON 10

//EEPROM locations
const unsigned char BAT_TYPE_STORE_LOC=0;
const unsigned char BAT_CELL_STORE_LOC=1;
#define BAT_TYPE_LIPO 0x10
#define BAT_TYPE_NIMH 0x20
#define BAT_TYPE_LEAD 0x30

//Initialise it for LIPO 2 Cell in EEPROM
__EEPROM_DATA(30,02,00,00,00,00,00,55);  

/*
 Based on 16V battery input range and a 10 bit ADC, the effective resolution is 15.625mV/bit
 The voltage seen at ADC input is 0.3125x the input voltage
 The Battery type analogue input uses the standard 4.88mV/bit resolution.
 * 
 */
// Might need to tweak single cell LiPO to use internal reference
#define SINGLE_LIPO_LOW     3.600     // 3.6V
#define SINGLE_LIPO_HIGH    4.250     // 4.25V
#define DUAL_LIPO_LOW       7.200     // 7.2V
#define DUAL_LIPO_HIGH      8.450     // 8.45V
#define TRIPLE_LIPO_LOW     10.800     // 10.8V
#define TRIPLE_LIPO_HIGH    12.600     // 12.6V

/* 
 Lead acid/AGM cell voltages
 * 100% 2.12-2.15V per cell
 * 20% 1.93V/cell
 * 0% 1.75V/cell
 * We will warn at 20% or 1.93V/cell
 */

#define LEAD_ACID_6V_LOW    5.790
#define LEAD_ACID_6V_HIGH   6.450    
#define LEAD_ACID_12V_LOW   11.580     
#define LEAD_ACID_12V_HIGH  12.900    


/* Might need to tweak 4 & 5 cell NiMh to use internal reference
 * Use 1.4V x number of cells for upper limit for battery detection
 * Use 1.2V for lower limit for detection, this is 50% charge for a cell
 * Use 1.0V x number of cells for lower limit 
 */

#define NIMH_4CELL_LOW      4.000    
#define NIMH_4CELL_DET_LOW  4.800
#define NIMH_4CELL_HIGH     5.600     

#define NIMH_5CELL_LOW      5.000    
#define NIMH_5CELL_DET_LOW  6.000
#define NIMH_5CELL_HIGH     7.000     

#define NIMH_6CELL_LOW      6.000     
#define NIMH_6CELL_DET_LOW  7.200
#define NIMH_6CELL_HIGH     8.400     

#define NIMH_7CELL_LOW      7.000     
#define NIMH_7CELL_DET_LOW  8.400
#define NIMH_7CELL_HIGH     9.600     

#define NIMH_8CELL_LOW      8.000     
#define NIMH_8CELL_DET_LOW  9.600
#define NIMH_8CELL_HIGH     11.200    








/*
 These voltages determine the battery type selected by the user.
 */

//#define DEBUG 12


#define BAT_NIMH_LOW        3.490 // 3.59V
#define BAT_NIMH_HIGH       4.070 // 3.97V
#define BAT_LIPO_LOW        2.275 // 2.375V
#define BAT_LIPO_HIGH       2.725 // 2.625V
#define BAT_PB_LOW          1.050 //1.15V
#define BAT_PB_HIGH         1.370 //1.27V

#define NUM_AVG 4

enum Bat_type {NIMH=10,PB=20,LIPO=30,UNKNOWN_BAT_TYPE=40};
enum Bat_config_type {SINGLE_LIPO=1,DUAL_LIPO=2,TRIPLE_LIPO=3, LEAD_ACID_6V=6,LEAD_ACID_12V=12,
NIMH_4CELL=40, NIMH_5CELL=50, NIMH_6CELL=60, NIMH_7CELL=70, NIMH_8CELL=80,UNKNOWN_CELL_COUNT=250};

enum Bat_type Battery_type=UNKNOWN_BAT_TYPE;
enum Bat_config_type Battery_Config=UNKNOWN_CELL_COUNT;
float LowerLimit=0,UpperLimit=1.0;
unsigned long Supply_Voltage, RefVoltage;
float ADC_Resn,vdd,BatteryVoltage,FBatSel; 



/*
 * Function to take NUM_AVG readings of the ADC and return the arithmetic mean
 * result. This should filter out noise
 
 */

//unsigned short Avg_ADC_GetConversion(unsigned short Channel)
float Avg_ADC_GetConversion(unsigned short Channel)
{
    unsigned short AveragedValue=0,AveragedResult=0;
    unsigned char ADCLoop;
    float ADCFloat=0.0;
    
    for (ADCLoop=0;ADCLoop<NUM_AVG;ADCLoop++)
    {
        AveragedValue+=ADC_GetConversion(Channel);
    }
    switch (NUM_AVG){
        case 2:
        {
            AveragedResult=AveragedValue >>1;
            break;
        }
        case 4:
        {
            AveragedResult=AveragedValue >>2;
            break;
        }
        case 8:
        {
            AveragedResult=AveragedValue >>3;
            break;
        }
    }
    if (Channel==BatSelect)
    {
        ADCFloat=(float)(ADC_Resn*AveragedResult);
        
    }
    else if (Channel==VBat)
    {
        ADCFloat=(float)(ADC_Resn/0.3125)*AveragedResult;
    }
    return (ADCFloat);
    }
 
/*
 * Function to read the voltage on the resistive divider and determine the battery
 * type. This is set by jumpers and the tolerances allow a ~5% variation for the 
 * onboard regulator.
 */

unsigned short DetermineBatType(void)
{
    FBatSel=Avg_ADC_GetConversion(BatSelect);
    
    if ((FBatSel>BAT_PB_LOW) && (FBatSel< BAT_PB_HIGH))
    {
        Battery_type=PB;
    }
    else if ((FBatSel>BAT_NIMH_LOW) && (FBatSel< BAT_NIMH_HIGH))
    {
        Battery_type=NIMH;
    }
    else if ((FBatSel>BAT_LIPO_LOW) && (FBatSel<BAT_LIPO_HIGH))
    {
        Battery_type=LIPO;
    }
    else
    {
        Battery_type=UNKNOWN_BAT_TYPE;
    }
#ifdef DEBUG
    Battery_type=NIMH;
#endif
    return(Battery_type);
}

unsigned char SoundBatAlarm(void)
{
#ifdef BUZZERON
    BUZZER=BUZZON;
    __delay_ms(500); //normally 300ms
    BUZZER=BUZZOFF;
    __delay_ms(500);
#else 
       LED1=LEDON;
       LED2=LEDON;
       __delay_ms(500);
       LED1=LEDOFF; 
       LED2=LEDOFF;
      __delay_ms(500);
#endif    
    return(1);
}

/*  Function to check the real time measured battery voltage, if outside of 
 * either test limit, we set Bealert to 1 to sound the alarm
 * 
 */

unsigned char CheckBatteryLevels(float MeasuredVoltage)
{
    unsigned char Bealert=0;
    float MeasVolt;
    MeasVolt=MeasuredVoltage;

    if((MeasVolt <LowerLimit) || (MeasVolt >UpperLimit))
	{
		Bealert=1;
	}
	else
	{
		Bealert=0;
	}
   return(Bealert);   
}

/*
 * Function to determine battery cell count based on measured voltage and type
 * The global BatteryVoltage, sampled before this function call, holds the current
 * calibrated battery voltage.
 */
unsigned char DetermineBatCellCount(void)
{
    unsigned short Batt_V=0;

    if (Battery_type == PB)
    {
        if (BatteryVoltage>=LEAD_ACID_6V_LOW && BatteryVoltage <= LEAD_ACID_6V_HIGH)
        {
            Battery_Config=LEAD_ACID_6V;
        }
        else if (BatteryVoltage>=LEAD_ACID_12V_LOW && BatteryVoltage <= LEAD_ACID_12V_HIGH)
        {
            Battery_Config=LEAD_ACID_12V;
        }
        else
        {
            Battery_Config=UNKNOWN_CELL_COUNT;
        }
    }
    else if (Battery_type == NIMH)
    {
        if (BatteryVoltage>=NIMH_4CELL_DET_LOW && BatteryVoltage <= NIMH_4CELL_HIGH )
        {
            Battery_Config= NIMH_4CELL;
        }
        else if (BatteryVoltage>=NIMH_6CELL_DET_LOW && BatteryVoltage <= NIMH_6CELL_HIGH )
        {
            Battery_Config= NIMH_6CELL;

        }
        else if (BatteryVoltage>=NIMH_8CELL_DET_LOW && BatteryVoltage <= NIMH_8CELL_HIGH )
        {
            Battery_Config= NIMH_8CELL;
          }
        else
        {
            Battery_Config=UNKNOWN_CELL_COUNT;
        }
    }
    
    else if (Battery_type == LIPO)
    {
        if (BatteryVoltage>=SINGLE_LIPO_LOW && BatteryVoltage <= SINGLE_LIPO_HIGH )
        {
            Battery_Config= SINGLE_LIPO;
        }
        else if (BatteryVoltage>=DUAL_LIPO_LOW && BatteryVoltage <= DUAL_LIPO_HIGH )
        {
            Battery_Config= DUAL_LIPO;
        }
        else if (BatteryVoltage>=TRIPLE_LIPO_LOW && BatteryVoltage <= TRIPLE_LIPO_HIGH )
        {
            Battery_Config= TRIPLE_LIPO;
        }
        else
        {
            Battery_Config=UNKNOWN_CELL_COUNT;
        }
        
    }
    
    return(Battery_Config);
}


unsigned int ReadFVR(void)
{
    unsigned int Vrefread=0;
    Vrefread=ADC_GetConversion(channel_FVR);
    return(Vrefread);
}

/*  Function to calibrate the ADC based on a read of the internal Fixed Voltage
 *  Reference (FVR) which is used to scale the power supply voltage, from this
 *  we determine the resolution of each ADC bit.
 */
unsigned char CalibrateADC(void)
{
  RefVoltage=ReadFVR();
    
/* Fixed point
    vdd = (65535 / SupplyVoltage) <<4;
*/    
    vdd = (float)((1024.0 /RefVoltage) * 1.024);
    ADC_Resn=vdd/1024;
}

/*  Function used to set the test limits for the battery. Providing the jumper
 * for battery type is correct, this should set the correct limits.
 */

unsigned char Set_Test_Limits(enum Bat_type Battery_Chemistry ,enum Bat_config_type Battery_Cell_Count)
{
    unsigned char numblinks=0;    
    unsigned char Cells=0;
    if (Battery_Chemistry == PB)
        {
        
            if (Battery_Cell_Count==LEAD_ACID_6V)
            {
                LowerLimit=LEAD_ACID_6V_LOW;
                UpperLimit=LEAD_ACID_6V_HIGH;
                numblinks=1;
            }
            else if (Battery_Cell_Count==LEAD_ACID_12V)
            {
                LowerLimit=LEAD_ACID_12V_LOW;
                UpperLimit=LEAD_ACID_12V_HIGH;
                numblinks=2;
            }
            
            for (Cells=0;Cells<numblinks;Cells++)
            {
                LED1=LEDON;
                __delay_ms(500);
                LED1=LEDOFF;
                __delay_ms(500);
            }
            //Now leave LED type indicator on throughout
            __delay_ms(1000);
            LED1=LEDON;
            LED2=LEDOFF;
        }
        
    else if (Battery_Chemistry == NIMH)
        {
            if (Battery_Cell_Count== NIMH_4CELL)
            {
                LowerLimit=NIMH_4CELL_LOW;
                UpperLimit=NIMH_4CELL_HIGH;
                numblinks=4;
            }
            else if (Battery_Cell_Count== NIMH_6CELL)
            {
                LowerLimit=NIMH_6CELL_LOW;
                UpperLimit=NIMH_6CELL_HIGH;
                numblinks=6;
            }
            else if (Battery_Cell_Count==NIMH_8CELL )
            {
                LowerLimit=NIMH_8CELL_LOW;
                UpperLimit=NIMH_8CELL_HIGH;
                numblinks=8;
            }
            for (Cells=0;Cells<numblinks;Cells++)
            {
                LED2=LEDON;
                __delay_ms(500);
                LED2=LEDOFF;
                __delay_ms(500);
            }
               
        //Permanent LED status
        __delay_ms(1000);
        LED1=LEDOFF;
        LED2=LEDON;
        }
    else if (Battery_Chemistry == LIPO)
    {
        if (Battery_Cell_Count==SINGLE_LIPO )
        {
            LowerLimit=SINGLE_LIPO_LOW;
            UpperLimit=SINGLE_LIPO_HIGH;
            numblinks=1;
        }
        else if (Battery_Cell_Count==DUAL_LIPO)
        {
            LowerLimit=DUAL_LIPO_LOW;
            UpperLimit=DUAL_LIPO_HIGH;
            numblinks=2;
        }
        else if (Battery_Cell_Count==TRIPLE_LIPO)
        {
            LowerLimit=TRIPLE_LIPO_LOW;
            UpperLimit=TRIPLE_LIPO_HIGH;
            numblinks=3;
        }
     
          for (Cells=0;Cells<numblinks;Cells++)
        {
             LED1=LEDON;
             LED2=LEDON;
            __delay_ms(500);
            LED1=LEDOFF; 
            LED2=LEDOFF;
            __delay_ms(500);
        }
          
        __delay_ms(1000);
        LED1=LEDON;
        LED2=LEDON;
    }
    return(1);
}


/*
                         Main application
 */
void main(void)
{
    unsigned char Alert=0;
    unsigned short RawBatVolt=0;
    unsigned char BatType_Stored=0,CellCount_Stored=99;

    // initialize the device
    SYSTEM_Initialize();
    // Ensure LEDs are off but sound buzzer for 500ms
    BUZZER=BUZZON;
    LED1=LEDOFF;
    LED2=LEDOFF;
    __delay_ms(500); 
    BUZZER=BUZZOFF;
    
    ADC_Initialize();
    CalibrateADC();
      
    
    //Recall settings from EEPROM, see if they match current settings
    BatType_Stored=eeprom_read(BAT_TYPE_STORE_LOC);
    CellCount_Stored=eeprom_read(BAT_CELL_STORE_LOC);  
    
    //Get a read of the battery to determine the type
    BatteryVoltage=Avg_ADC_GetConversion(VBat);
    DetermineBatType(); // Determine type and flash LEDs
    //__delay_ms(500);            
    Battery_Config=DetermineBatCellCount();
    
      /*If current settings do not match those stored, update the stored settings 
      and set the cell count. 
     */
    if(BatType_Stored!=Battery_type)
    {
      eeprom_write(BAT_TYPE_STORE_LOC,Battery_type);
      eeprom_write(BAT_CELL_STORE_LOC,Battery_Config); 
    }
    
    /* This is to capture a flat battery and sound the alarm, based on the stored settings.
     * A flat battery should return a UNKNOWN_CELL_COUNT so we used the saved
     * settings from the EEPROM.
     */
    if (Battery_Config==UNKNOWN_CELL_COUNT)
    {
        Battery_Config=CellCount_Stored;
    }
    
    Set_Test_Limits(Battery_type,Battery_Config);

    while (1)
    {
        BatteryVoltage=Avg_ADC_GetConversion(VBat);
        Alert=CheckBatteryLevels(BatteryVoltage);
        if (Alert==1)
        {
            SoundBatAlarm();
        }
        __delay_ms(25);
        
    }
}
/**
 End of File
*/