#include "MAX30102.h"

uint32_t IR_Buffer[500];
uint32_t RED_Buffer[500];
int32_t IR_Buffrt_Length;

void Max30102_Reset(void)
{
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,mode_config_rigister,0x40,1);         //RESET Bit To 1
}

void Max30102_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_InitStruct.GPIO_Pin = MAX30102_IT_Pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MAX30102_IT_Port,&GPIO_InitStruct);

    I2C_Config();
    Max30102_Reset();

    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,interrupt_enable_1_rigister,0xC0,1);  //Enable A_FULL and PPG_RDY
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,fifo_wr_ptr_rigister,0x00,1);         //Write_prt reset
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,over_flow_cnt_rigister,0x00,1);       //over_flow_ptr reset
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,fifo_rd_ptr_rigister,0x00,1);         //read_ptr reset
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,fifo_config_rigister,0x07,1);         //No average,No Rolls,Almost Full = 17 
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,mode_config_rigister,0x03,1);         //Heart Rate and SPO2 Mode
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,spO2_config_rigister,0x27,1);         //SpO2 ADC Range=4096nA,SPO2 Rate=100Hz,Pulse Width=400ns
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,led1_pulse_amplitude_rigister,0x24,1);//almost 7mA for LED1
    I2C_write_OneByte(MAX30102_I2C,write_slave_addr,led2_pulse_amplitude_rigister,0x24,1);//almost 7mA for LED2
}

void Max30102_Read_FIFO(uint32_t *RED,uint32_t *IR)
{
    uint8_t IT_Status;
    uint32_t red_temp,ir_temp;
    uint8_t data_array[6];
    I2C_read_Bytes(MAX30102_I2C,read_slave_addr,interrupt_status_1_rigister,&IT_Status,1); //clead IT IT_Status

    I2C_read_Bytes(MAX30102_I2C,read_slave_addr,fifo_data_rigister,data_array,6);

    red_temp = (data_array[0] << 16) + (data_array[1] << 8) + data_array[2];
    ir_temp  = (data_array[3] << 16) + (data_array[4] << 8) + data_array[5];

    *RED = red_temp & 0x03ffff; //18bit
    *IR  = ir_temp  & 0x03ffff; //18bit
}

void Max30102_Calculate(uint32_t *RED,uint32_t *IR,int32_t *SPO2_Value,int32_t *HR_Value)
{
    int i;
    //float f_temp;
    int8_t SPO2_Valid;
    int8_t HR_Valid;
    //int32_t brightness;
    uint32_t min_data=0;
    uint32_t max_data=0x3ffff;
    //uint32_t pre_data;

    //uint32_t min1_data=0x3ffff;
    //uint32_t max1_data=0;

    IR_Buffrt_Length = 500;
    for(i=0;i<IR_Buffrt_Length;i++){
        while(MAX30102_IT_STATUS() == 1); //until Intertupt assert

        Max30102_Read_FIFO(RED,IR); //read FIFO data
        IR_Buffer[i]  = *IR;
        RED_Buffer[i] = *RED;

        if(min_data>RED_Buffer[i])          //updata min and max data
            min_data = RED_Buffer[i];
        if(max_data < RED_Buffer[i])
            max_data = RED_Buffer[i];
    }
    //pre_data = RED_Buffer[i];

    //calculate heart rate and SpO2 after first 500 samples (first 5 seconds of samples)
    maxim_heart_rate_and_oxygen_saturation(IR_Buffer,IR_Buffrt_Length,RED_Buffer,SPO2_Value,&SPO2_Valid,HR_Value,&HR_Valid);

    if(!(HR_Valid==1 && (*HR_Value)<120)){
        *HR_Value = 0;
        *SPO2_Value = 0;
    }
    
/*
    //dumping the first 100 sets of samples in the memory and shift the last 400 sets of samples to the top
    for(i=100;i<500;i++){
        RED_Buffer[i-100] = RED_Buffer[i];
        IR_Buffer[i-100] = IR_Buffer[i];
        //update the signal min and max
        if(min1_data > RED_Buffer[i])
            min1_data = RED_Buffer[i];
        if(max1_data < RED_Buffer[i])
            max1_data = RED_Buffer[i];
    }
    //take 100 sets of samples before calculating the heart rate.
    for(i=400;i<500;i++){
        pre_data = RED_Buffer[i-1];
        while(MAX30102_IT_STATUS() == 1); //until Intertupt assert
        Max30102_Read_FIFO(RED,IR); //read FIFO data
        IR_Buffer[i]  = *IR;
        RED_Buffer[i] = *RED;

        if(RED_Buffer[i] > pre_data){
            f_temp = RED_Buffer[i] - pre_data;
            f_temp /= (max1_data-min1_data);
            f_temp *= MAX_BRIGHTNESS;
            brightness -= (int)f_temp;
            if(brightness<0)
                brightness = 0;
        }else{
            f_temp = pre_data-RED_Buffer[i];
            f_temp /= (max1_data-min1_data);
            f_temp *= MAX_BRIGHTNESS;
            brightness += (int)f_temp;
            if(brightness>MAX_BRIGHTNESS)
                brightness = MAX_BRIGHTNESS;
        }
    }
*/
}