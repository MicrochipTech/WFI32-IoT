#include <stdio.h>

#include "OLEDB.h"
#include "driver/spi/drv_spi.h"
#include "system/ports/sys_ports.h"
#include "system/time/sys_time.h"
#include "peripheral/spi/spi_master/plib_spi2_master.h"
#include "system/console/sys_console.h"


static OLEDB_DATA oledData;


uint8_t OLED_B_LCDWIDTH = 96;
uint8_t OLED_B_LCDHEIGHT = 39;
uint8_t OLED_B_DISPLAYOFF = 0xAE;
uint8_t OLED_B_SETDISPLAYCLOCKDIV = 0xD5;
uint8_t OLED_B_SETMULTIPLEX = 0xA8;
uint8_t OLED_B_SETDISPLAYOFFSET = 0xD3;
uint8_t OLED_B_SETSTARTLINE = 0x40;
uint8_t OLED_B_CHARGEPUMP = 0x8D;
uint8_t OLED_B_SETSEGMENTREMAP = 0xA1;
uint8_t OLED_B_SEGREMAP = 0xA0;
uint8_t OLED_B_COMSCANDEC = 0xC8;
uint8_t OLED_B_SETCOMPINS = 0xDA;
uint8_t OLED_B_SETCONTRAST = 0x81;
uint8_t OLED_B_SETPRECHARGE = 0xD9;
uint8_t OLED_B_SETVCOMDETECT = 0xDB;
uint8_t OLED_B_DISPLAYALLON_RESUME = 0xA4;
uint8_t OLED_B_NORMALDISPLAY = 0xA6;
uint8_t OLED_B_DISPLAYON = 0xAF;
uint8_t OLED_B_DISPLAYALLON = 0xA5;
uint8_t OLED_B_INVERTDISPLAY = 0xA7;
uint8_t OLED_B_SETLOWCOLUMN = 0x00;
uint8_t OLED_B_SETHIGHCOLUMN = 0x10;
uint8_t OLED_B_MEMORYMODE = 0x20;
uint8_t OLED_B_COLUMNADDR = 0x21;
uint8_t OLED_B_PAGEADDR = 0x22;
uint8_t OLED_B_COMSCANINC = 0xC0;
uint8_t OLED_B_EXTERNALVCC = 0x1;
uint8_t OLED_B_SWITCHCAPVCC = 0x2;
uint8_t OLED_B_ACTIVATE_SCROLL = 0x2F;
uint8_t OLED_B_DEACTIVATE_SCROLL = 0x2E;
uint8_t OLED_B_SET_VERTICAL_SCROLL_AREA = 0xA3;
uint8_t OLED_B_RIGHT_HORIZONTAL_SCROLL = 0x26;
uint8_t OLED_B_LEFT_HORIZONTAL_SCROLL = 0x27;
uint8_t OLED_B_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL = 0x29;
uint8_t OLED_B_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL = 0x2A;
uint8_t OLED_B_ADDR_COMMAND = 0x3C;
uint8_t OLED_B_ADDR_DATA = 0x3D;

void delay_ms(uint32_t delay) {
    vTaskDelay(delay / portTICK_PERIOD_MS);
}

bool oledb_sendCommandWAck(uint8_t wData) {
    uint8_t rData;
    bool ret = false;
    SPI2_CS_Clear();
    PWM_Clear();

    if (DRV_SPI_WriteReadTransfer(oledData.spiHandle, &wData, 1,&rData, 1) == false) {
        SYS_CONSOLE_PRINT("\r\nDRV_SPI_WriteTransfer failed\r\n");
    }
    else if(wData == rData)
        ret = true;
    
    SPI2_CS_Set();
    return ret;
}

void oledb_sendCommand(uint8_t wData) {
    SPI2_CS_Clear();
    PWM_Clear();

    if (DRV_SPI_WriteTransfer(oledData.spiHandle, &wData, 1) == false) {
        SYS_CONSOLE_PRINT("\r\nDRV_SPI_WriteTransfer failed\r\n");
    }
    SPI2_CS_Set();
}

void oledb_sendData(uint8_t wData) {
    SPI2_CS_Clear();
    PWM_Set();

    if (DRV_SPI_WriteTransfer(oledData.spiHandle, &wData, 1) == false) {
        SYS_CONSOLE_PRINT("\r\nDRV_SPI_WriteTransfer failed\r\n");
    }
    SPI2_CS_Set();
}

void oledb_init(bool* loopback) {
    RST_Clear();
    delay_ms(1000);
    RST_Set();
    delay_ms(1000);
    *loopback = oledb_sendCommandWAck(OLED_B_DISPLAYOFF); //0xAE Set OLED Display Off
    oledb_sendCommand(OLED_B_SETDISPLAYCLOCKDIV); //0xD5 Set Display Clock Divide Ratio/Oscillator Frequency
    oledb_sendCommand(0x80);
    oledb_sendCommand(OLED_B_SETMULTIPLEX); //0xA8 Set Multiplex Ratio
    oledb_sendCommand(0x27);
    oledb_sendCommand(OLED_B_SETDISPLAYOFFSET); //0xD3 Set Display Offset
    oledb_sendCommand(0x00);
    oledb_sendCommand(OLED_B_SETSTARTLINE); //0x40 Set Display Start Line
    oledb_sendCommand(OLED_B_CHARGEPUMP); //0x8D Set Charge Pump
    oledb_sendCommand(0x14); //0x14 Enable Charge Pump

    oledb_sendCommand(OLED_B_COMSCANDEC); //0xC8 Set COM Output Scan Direction
    oledb_sendCommand(OLED_B_SETCOMPINS); //0xDA Set COM Pins Hardware Configuration
    oledb_sendCommand(0x12);
    oledb_sendCommand(OLED_B_SETCONTRAST); //0x81 Set Contrast Control
    oledb_sendCommand(0xAF);

    oledb_sendCommand(OLED_B_SETPRECHARGE); //0xD9 Set Pre-Charge Period
    oledb_sendCommand(0x25);
    oledb_sendCommand(OLED_B_SETVCOMDETECT); //0xDB Set VCOMH Deselect Level
    oledb_sendCommand(0x20);
    oledb_sendCommand(OLED_B_DISPLAYALLON_RESUME); //0xA4 Set Entire Display On/Off
    oledb_sendCommand(OLED_B_NORMALDISPLAY); //0xA6 Set Normal/Inverse Display

    oledb_sendCommand(OLED_B_DISPLAYON); //0xAF Set OLED Display On
}

//Set page adress for Page Addressing Mode

void oledb_setPage(uint8_t addr) {
    addr = 0xb0 | addr;
    oledb_sendCommand(addr);
}

//Set column adress for Page Addressing Mode

void oledb_setColumn(uint8_t addr) {
    oledb_sendCommand((OLED_B_SETHIGHCOLUMN | (addr >> 4)));
    oledb_sendCommand((0x0f & addr));
}

//Display picture for Page Addressing Mode

void oledb_clearDisplay(void) {
    uint8_t i;
    uint8_t j;
    if (oledData.status == true){
        for (i = 0; i < 0x05; i++) {
            oledb_setPage(i);
            // Set_Column_Address(0x00);
            oledb_sendCommand(OLED_B_SETHIGHCOLUMN);
            oledb_sendCommand(OLED_B_SETSTARTLINE);
            for (j = 0; j < 96; j++) {
                oledb_sendData(0x00);
            }
        }
    }
}

void oledb_displayOff(void) {
    oledb_sendCommand(OLED_B_DISPLAYOFF); //0xAE Set OLED Display Off
}

void oledb_displayOn(void) {
    oledb_sendCommand(OLED_B_DISPLAYON); //0xAF Set OLED Display On
}

// an ugly display driver. /r/programminghorror
void oledb_displayXY(uint8_t x, uint8_t y) {
    if(oledData.status){
        uint8_t i=0;
        uint8_t font_num[] = {
            0x3E, 0x45, 0x49, 0x51, 0x3E, //0
            0x00, 0x40, 0x7F, 0x42, 0x00, //1
            0x46, 0x49, 0x49, 0x49, 0x72, //2
            0x33, 0x4D, 0x49, 0x41, 0x21, //3
            0x10, 0x7F, 0x12, 0x14, 0x18, //4
            0x39, 0x45, 0x45, 0x45, 0x27, //5
            0x31, 0x49, 0x49, 0x4A, 0x3C, //6
            0x07, 0x09, 0x11, 0x21, 0x41, //7
            0x36, 0x49, 0x49, 0x49, 0x36, //8
            0x1E, 0x29, 0x49, 0x49, 0x46, //9
        };

        uint8_t font_X[]={0x63, 0x14, 0x08, 0x14, 0x63,}; //X

        oledb_clearDisplay();

        oledb_setPage(2);
        oledb_sendCommand(OLED_B_SETHIGHCOLUMN);
        oledb_sendCommand(OLED_B_SETSTARTLINE);

        for(i=0;i<25;i++){
           oledb_sendData(0); //5 space chars
        }

        for (i = 0; i<5; i++) {
            oledb_sendData(font_num[(5*y)+i]);
        }

        for (i = 0; i<5; i++){
            oledb_sendData(0);
        }

        for (i = 0; i<5; i++){
            oledb_sendData(font_X[i]);
        }

        for (i = 0; i<5; i++){
            oledb_sendData(0);
        }

        for (i = 0; i<5; i++) {
            oledb_sendData(font_num[(5*x)+i]);
        }
    }
}

void oledb_displayPicture(const uint8_t *pic) {
    uint8_t i;
    uint8_t j;
    if (oledData.status == true){
        for (i = 0; i < 0x05; i++) {
            oledb_setPage(i);
            // Set_Column_Address(0x00);
            oledb_sendCommand(OLED_B_SETHIGHCOLUMN);
            oledb_sendCommand(OLED_B_SETSTARTLINE);
            for (j = 0; j < 96; j++) {
                oledb_sendData(pic[i * 96 + j]);
            }
        }
    }
}

void oledb_setContrast(uint8_t temp) {
    oledb_sendCommand(OLED_B_SETCONTRAST); // 0x81   Set Contrast Control
    oledb_sendCommand(temp); // contrast step 1 to 256
}

int oledb_initialize(bool* loopback) {
    oledData.status=false;
    oledData.spiHandle = DRV_SPI_Open(DRV_SPI_INDEX_0, DRV_IO_INTENT_EXCLUSIVE);
    if (oledData.spiHandle == DRV_HANDLE_INVALID) {
          return -1; //failed opening SPI driver
    }
    else{
            oledb_init(loopback);
            delay_ms(500);
            oledData.status=true;
            oledb_clearDisplay();
            return 0;
    }
}