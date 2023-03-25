#include "ESP8266.h"

extern uint8_t DMA_RCV_Buffer[DMA_SIZE];
char Pub_Data[PUB_SIZE];
uint16_t RCV_CNT=0;
uint8_t ESP_STATE;

uint8_t AT_CIPSNTPCFG[]="AT+CIPSNTPCFG=1,8,\"ntp1.aliyun.com\"\r\n";    //设置地区时间连接阿里云

uint8_t AT_WIFI_INFO[]="AT+CWJAP=\"gidle\",\"2161826815\"\r\n";   //连接WIFI

//设置MQTT用户
uint8_t AT_MQTTUSRCFG[]="AT+MQTTUSERCFG=0,1,\"NULL\",\"ESP8266&i5z42JpfDlV\",\"F153B43A65CC96889F0CEBF96984EE1993682453\",0,0,\"\"\r\n";

//设置MQTT客户端
uint8_t AT_MQTTCLIENTID[]="AT+MQTTCLIENTID=0,\"2161826815|securemode=2\\,signmethod=hmacsha1\\,timestamp=1676045469270|\"\r\n";

//以MQTT协议连接阿里云
uint8_t AT_MQTTCONN[]="AT+MQTTCONN=0,\"i5z42JpfDlV.iot-as-mqtt.cn-shanghai.aliyuncs.com\",1883,1\r\n";

uint8_t AF_MQTTPUB_TEMP[]="AT+MQTTPUB=0,\"/i5z42JpfDlV/ESP8266/user/temperature\",\"xxx\",1,0\r\n";  //发送温度值到云端，xxx需要格式化
uint8_t AF_MQTTPUB_HR_SPO2[]="AT+MQTTPUB=0,\"/i5z42JpfDlV/ESP8266/user/HR_SPO2\",\"xxx\",1,0\r\n";  //发送心率血氧值到云端，xxx需要格式化


//清空接收数组
void ESP8266_RCV_Clear(void)
{
    memset(DMA_RCV_Buffer,0,DMA_SIZE);
    RCV_CNT = 0;
}

//成功返回0.失败返回1
uint8_t ESP8266_Wait(void)
{  
    if(RCV_CNT == 0)
        return 1;
    else
        return 0;
}


static void delay()
{
    uint32_t i = 0xfffff;
    while(i--);
}

//发送AT指令，成功返回0，失败返回1
uint8_t ESP8266_Send_Cmd(uint8_t* cmd,const char* ret)
{
    uint16_t timeout = 500;
    while(timeout--){
        USART_Send_str(ESP8266_USARTX,cmd);
        //printf("DMA_RCV_Buffer:  %s,times:%d\r\n",DMA_RCV_Buffer,timeout);
        if(strstr((const char*)DMA_RCV_Buffer,ret) != NULL){
            ESP8266_RCV_Clear();
            printf("Recieve OK\r\n");
            return 0;
        }
        delay();
          
        //delay_ms(1000);
    }
    return 1;
}

//ESP8266初始化 成功返回0，失败返回对应值
uint8_t ESP8266_Init()
{
    uint8_t ret;
    //uint8_t timeout = 10;
    ESP8266_GPIO_Config();
/*
    ret = ESP8266_Send_Cmd("AT\r\n","OK");      //测试是否正常工作
    if(ret != 0)
       return 1;
*/

/*
    ret = ESP8266_Send_Cmd("AT+RST\r\n","OK");  //复位
    if(ret != 0)
       return 2;
*/
/*
    ret = ESP8266_Send_Cmd("ATE0\r\n","OK");    //关闭回显
    if(ret != 0)
       return 3;
*/
/*
    ret = ESP8266_Send_Cmd("AT+CWMODE=3\r\n","OK");  //Station+AP模式
    if(ret != 0)
       return 4;

    ret = ESP8266_Send_Cmd(AT_CIPSNTPCFG,"OK");  //设置地区时间连接阿里云
    if(ret != 0)
       return 5;

    ret = ESP8266_Send_Cmd(AT_WIFI_INFO,"OK");  //连接WIFI
    if(ret != 0)
       return 6;

    ret = ESP8266_Send_Cmd(AT_MQTTUSRCFG,"OK");  //设置MQTT用户
    if(ret != 0)
       return 7;

    ret = ESP8266_Send_Cmd(AT_MQTTCLIENTID,"OK");  //设置MQTT客户端
    if(ret != 0)
       return 8;

    ret = ESP8266_Send_Cmd(AT_MQTTCONN,"OK");  //连接阿里云
    if(ret != 0)
       return 9;
*/
    
    return 0;
}

void ESP8266_Pub_Data(uint8_t data,int type)
{
    switch(type){
    case Type_Temperature:
        sprintf(Pub_Data,"AT+MQTTPUB=0,\
\"/sys/i5z42JpfDlV/ESP8266/thing/event/property/post\",\
\"{\"method\":\"thing.service.property.set\",\
\"id\":\"1\",\
\"params\":{\
\"Temperature\":%f\
},\
\"version\":\"1.0\"}\",1,0\r\n",data);

        //printf("%s",Pub_Data);
        if(ESP8266_Send_Cmd((uint8_t*)Pub_Data,"OK") == 0){
            printf("Pub data Success\r\n");
        }else{
            printf("Pub Data Fail\r\n");
        }
        break;
    case Type_HR:
        sprintf(Pub_Data,"AT+MQTTPUB=0,\
\"/sys/i5z42JpfDlV/ESP8266/thing/event/property/post\",\
\"{\"method\":\"thing.service.property.set\",\
\"id\":\"456\",\
\"params\":{\
\"HeartRate\":%d\
},\
\"version\":\"1.0\"}\",1,0\r\n",data);

        printf("%s",Pub_Data);
        if(ESP8266_Send_Cmd((uint8_t*)Pub_Data,"OK") == 0){
            printf("Pub data Success\r\n");
        }else{
            printf("Pub Data Fail\r\n");
        }
        break;
       
    case Type_SPO2:
        sprintf(Pub_Data,"AT+MQTTPUB=0,\
\"/sys/i5z42JpfDlV/ESP8266/thing/event/property/post\",\
\"{\"method\":\"thing.service.property.set\",\
\"id\":\"2\",\
\"params\":{\
\"SPO2\":%d\
},\
\"version\":\"1.0\"}\",1,0\r\n",data);

        //printf("%s",Pub_Data);
        if(ESP8266_Send_Cmd((uint8_t*)Pub_Data,"OK") == 0){
            printf("Pub data Success\r\n");
        }else{
            printf("Pub Data Fail\r\n");
        }  
        break;

    default:
        break;
    }
    memset(Pub_Data,0,sizeof(uint8_t)*PUB_SIZE);
}

/*
AT+MQTTPUB=0,"/sys/i5z42JpfDlV/ESP8266/thing/event/property/post","{\"method\":\"thing.service.property.set\"\,\"id\":\"1\"\,\"params\":{\"SPO2\":55}\,\"version\":\"1.0\"}",1,0
*/

/*
AT+MQTTPUB=0,"/sys/i5z42JpfDlV/ESP8266/thing/event/property/post","{\"method\":\"thing.service.property.set\"\,\"id\":\"1\"\,\"params\":{"HeartRate\":66}\,\"version\":\"1.0\"}",1,0
*/

/*
AT+MQTTPUB=0,"/sys/i5z42JpfDlV/ESP8266/thing/event/property/post","{\"method\":\"thing.service.property.set\"\,\"id\":\"1\"\,\"params\":{"Temperature\":77.7}\,\"version\":\"1.0\"}",1,0
*/
