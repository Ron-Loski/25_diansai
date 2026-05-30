import os

path = r'D:\diansai2025\DAC904_HighSpeedDAC_V1.6-SinExample\UART\UART\uart_test\Core\Src\main.c'

with open(path, 'rb') as f:
    raw = f.read()

# Find the start of UART1_SendByte (before FPGA_Send_Freq) and delay_ms function
start_marker = b'void UART1_SendByte'
end_marker = b'void delay_ms'

old_start = raw.find(start_marker)
old_end = raw.find(end_marker, old_start)

if old_start == -1 or old_end == -1:
    print("FAIL: markers not found")
    exit(1)

new_code = b'''// UART send byte (blocking)
static void UART1_SendByte(uint8_t byte)
{
    HAL_UART_Transmit(&huart1, &byte, 1, 0xFFFF);
}

// Generic command frame sender
static void FPGA_Send_Cmd(uint8_t cmd, uint32_t data)
{
    uint8_t buf[7];

    buf[0] = 0xA5;
    buf[1] = cmd;
    buf[2] = (data >>  0) & 0xFF;
    buf[3] = (data >>  8) & 0xFF;
    buf[4] = (data >> 16) & 0xFF;
    buf[5] = (data >> 24) & 0xFF;
    buf[6] = buf[0] ^ buf[1] ^ buf[2] ^ buf[3] ^ buf[4] ^ buf[5];

    for (uint8_t i = 0; i < 7; i++)
    {
        UART1_SendByte(buf[i]);
    }
}

// Set DDS frequency
void FPGA_Send_Freq(uint32_t freq_word)
{
    FPGA_Send_Cmd(0x01, freq_word);
}

// Set DDS amplitude, permille=0~1000 => 0.0%~100.0%
void FPGA_Set_Amp(uint16_t permille)
{
    uint32_t amp;

    if (permille > 1000)
        permille = 1000;

    amp = ((uint32_t)permille * 1024 + 500) / 1000;
    FPGA_Send_Cmd(0x03, amp);
}

'''

result = raw[:old_start] + new_code + b'\r\n' + raw[old_end:]

with open(path, 'wb') as f:
    f.write(result)

print("Done")
