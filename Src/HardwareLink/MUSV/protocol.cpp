#include "protocol.h"
#include "QString"
#include "QMessageBox"
#include "QDebug"
using namespace protocolName;

protocol::protocol()
{
    data.Encoders[0] = 0;
    data.Encoders[1] = 0;
    data.Encoders[2] = 0;
    data.newEncoders = true;
    data.newOther = true;


    data.LdDistance = 0;
    data.LdTemperature = 0;
    data.newLd = true;


    data.angles.roll = 0.f;
    data.angles.pitch = 0.f;
    data.angles.yaw = 0.f;
    data.newAngles = false;

    data.newPid.roll = false;
    data.pid.roll.Kp = 0.f;
    data.pidFlag.roll.Kp = false;
    data.pid.roll.Lp = 0.f;
    data.pidFlag.roll.Lp = false;
    data.pid.roll.Ki = 0.f;
    data.pidFlag.roll.Ki = false;
    data.pid.roll.Li = 0.f;
    data.pidFlag.roll.Li = false;
    data.pid.roll.Kd = 0.f;
    data.pidFlag.roll.Kd = false;
    data.pid.roll.Ld = 0.f;
    data.pidFlag.roll.Ld = false;
    data.pid.roll.L = 0.f;
    data.pidFlag.roll.L = false;

    data.newPid.pitch = false;
    data.pid.pitch.Kp = 0.f;
    data.pidFlag.pitch.Kp = false;
    data.pid.pitch.Lp = 0.f;
    data.pidFlag.pitch.Lp = false;
    data.pid.pitch.Ki = 0.f;
    data.pidFlag.pitch.Ki = false;
    data.pid.pitch.Li = 0.f;
    data.pidFlag.pitch.Li = false;
    data.pid.pitch.Kd = 0.f;
    data.pidFlag.pitch.Kd = false;
    data.pid.pitch.Ld = 0.f;
    data.pidFlag.pitch.Ld = false;
    data.pid.pitch.L = 0.f;
    data.pidFlag.pitch.L = false;

    data.newPid.yaw = false;
    data.pid.yaw.Kp = 0.f;
    data.pidFlag.yaw.Kp = false;
    data.pid.yaw.Lp = 0.f;
    data.pidFlag.yaw.Lp = false;
    data.pid.yaw.Ki = 0.f;
    data.pidFlag.yaw.Ki = false;
    data.pid.yaw.Li = 0.f;
    data.pidFlag.yaw.Li = false;
    data.pid.yaw.Kd = 0.f;
    data.pidFlag.yaw.Kd = false;
    data.pid.yaw.Ld = 0.f;
    data.pidFlag.yaw.Ld = false;
    data.pid.yaw.L = 0.f;
    data.pidFlag.yaw.L = false;

    data.newCalib = false;
}


float protocol::ConvertValue(QString str, bool* ok)
{
    str.replace(",",".");
    float data = str.toFloat(ok);
    return data;
}

QByteArray protocol::GenerateMessage(u_int8_t typeMessage, u_int8_t typeReg, QString str)
{
    bool ok;
    float value = ConvertValue(str, &ok);
    if(ok)
        return GenerateMessage(typeMessage, typeReg, value);
    else
        return QByteArray(0);
}

QByteArray protocol::GenerateMessage(u_int8_t typeMessage, u_int8_t typeReg, float value)
{
    u_int8_t size = 8;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = typeMessage;
    sum += typeMessage;
    data[1] = size;
    sum += size;
    data[2] = typeReg;
    sum += typeReg;
    ValueToArray(value, data+3, &sum);
    data[7] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateMessage(u_int8_t typeMessage, u_int8_t typeReg, u_int16_t value)
{
    u_int8_t size = 6;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = typeMessage;
    sum += typeMessage;
    data[1] = size;
    sum += size;
    data[2] = typeReg;
    sum += typeReg;
    data[3] = value;
    sum += value;
    data[4] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateMessage(u_int8_t typeMessage, u_int8_t typeReg, u_int8_t value)
{
    u_int8_t size = 5;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = typeMessage;
    sum += typeMessage;
    data[1] = size;
    sum += size;
    data[2] = typeReg;
    sum += typeReg;
    data[3] = value;
    sum += value;
    data[4] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateMessage(u_int8_t typeMessage, u_int8_t typeReg, u_int8_t value1, u_int16_t value2)
{
    u_int8_t size = 7;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = typeMessage;
    sum += typeMessage;
    data[1] = size;
    sum += size;
    data[2] = typeReg;
    sum += typeReg;
    data[3] = value1;
    sum += value1;
    ValueToArray(value2, data+4, &sum);
    data[6] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateAngles(float value1, float value2, float value3)
{
    u_int8_t size = 15;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = MUSV_ANGLES;
    sum += MUSV_ANGLES;
    data[1] = size;
    sum += size;
    ValueToArray(value1, data+2, &sum);
    ValueToArray(value2, data+6, &sum);
    ValueToArray(value3, data+10, &sum);
    data[14] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateCalib(float value1, float value2, float value3,
                                   float value4, float value5, float value6)
{
    u_int8_t size = 28;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = MUSV_SERVICE;
    sum += MUSV_SERVICE;
    data[1] = size;
    sum += size;
    data[2] = MUSV_SERVICE_CALIB_DATA;
    sum += MUSV_SERVICE_CALIB_DATA;
    ValueToArray(value1, data+3, &sum);
    ValueToArray(value2, data+7, &sum);
    ValueToArray(value3, data+11, &sum);
    ValueToArray(value4, data+15, &sum);
    ValueToArray(value5, data+19, &sum);
    ValueToArray(value6, data+23, &sum);
    data[27] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateMessage(u_int8_t typeMessage)
{
    u_int8_t size = 3;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = typeMessage;
    sum += typeMessage;
    data[1] = size;
    sum += size;
    data[2] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateMessage(u_int8_t typeMessage, u_int8_t typeReg)
{
    u_int8_t size = 4;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = typeMessage;
    sum += typeMessage;
    data[1] = size;
    sum += size;
    data[2] = typeReg;
    sum += typeReg;
    data[3] = sum;
    message.append(data, size);
    return message;
}

QByteArray protocol::GenerateMagTransform(float *Trans, float *Bias)
{
    u_int8_t size = 52;
    u_int8_t sum = 0;
    char data[size];
    QByteArray message;
    data[0] = MUSV_SETTINGS;
    data[1] = size;
    data[2] = MSG_DATA_CALIBRATION_DATA;
    for (int i = 0; i < 9; i++) {
        ValueToArray(*(Trans+i), data+3+4*i, &sum);
    }
    for (int i = 0; i < 3; i++) {
        ValueToArray(*(Bias+i), data+39+4*i, &sum);
    }
    sum = 0;
    for (int i = 0; i< size - 1; i++)
        sum += data[i];
    data[size - 1] = sum;
    message.append(data, size);
    return message;
}

void protocol::ValueToArray(float value, char* array, u_int8_t* sum)
{
    u_int8_t temp;
    char* data = (char*) &value;
    memcpy(array, data, 4);
    temp = array[0];
    *sum += array[0];
    array[0] = array[3];
    *sum += array[3];
    array[3] = temp;
    temp = array[1];
    *sum += array[1];
    array[1] = array[2];
    *sum += array[2];
    array[2] = temp;
}

void protocol::ValueToArray(u_int16_t value, char* array, u_int8_t* sum)
{
    u_int8_t temp;
    char* data = (char*) &value;
    memcpy(array, data, 2);
    temp = array[0];
    *sum += array[0];
    array[0] = array[1];
    *sum += array[1];
    array[1] = temp;
}

float protocol::ByteToFloat(u_int8_t index)
{
    float value;
    u_int8_t temp;
    temp = array[index];
    array[index] = array[index+3];
    array[index+3] = temp;
    temp = array[index+1];
    array[index+1] = array[index+2];
    array[index+2] = temp;

    memcpy(&value, array.data()+index, 4);
    return value;
}

void protocol::SetData(QByteArray arr)
{
    array += arr;
}

void protocol::DecodingData()
{
    int newIndex = -1;
    int lastIndex;
    do
    {
        lastIndex = newIndex;
        newIndex = FindNextHead(lastIndex+1);
        if (newIndex >= 0)
        {
            u_int8_t dataLength = (u_int8_t)array[newIndex + 1];

            if(CheckSum(newIndex) == true)
            {

                switch(array[newIndex])
                {
                case MUSV_IN_ANGLES:
                    if (dataLength != 15) break;
                    data.angles.roll = ByteToFloat(newIndex+2);
                    data.angles.pitch = ByteToFloat(newIndex+6);
                    data.angles.yaw = ByteToFloat(newIndex+10);
                    data.newAngles = true;
                    break;
                case MUSV_IN_ENCODERS:
                    if (dataLength != 15) break;
                    data.Encoders[0] = ByteToFloat(newIndex+2);
                    data.Encoders[1] = ByteToFloat(newIndex+6);
                    data.Encoders[2] = ByteToFloat(newIndex+10);
                    data.newEncoders = true;
                    break;
                case MUSV_IN_LD_T_D:
                    if (dataLength != 11) break;
                    data.LdTemperature = ByteToFloat(newIndex+2);
                    data.LdDistance = ByteToFloat(newIndex+6);
                    data.newLd = true;
                    break;
                case MUSV_IN_OTHER:
                    if (dataLength != 15) break;
                    data.ZoomAll = array[newIndex + 2];
                    data.ZoomO = array[newIndex + 3];
                    data.ZoomD = array[newIndex + 4];
                    data.newOther = true;
                    break;
                case MSG_TYPE_OUT_BINS:
                    for (int i = 0; i < 3; ++i) {
                        data.Accel[i]  = ByteToFloat(newIndex+2+i*4);
                        data.Gyro[i]  = ByteToFloat(newIndex+14+i*4);
                        data.Mag[i]  = ByteToFloat(newIndex+26+i*4);
                    }
                    data.Temp =  ByteToFloat(newIndex+38);
                    data.Time =  ByteToFloat(newIndex+42);
                    data.newBins = true;
                    break;
                case MUSV_IN_SERVICE:
                    switch(array[newIndex+2])
                    {
                    //Roll
                    case MUSV_SERVICE_PID_ROLL_Kp:
                        data.pid.roll.Kp = ByteToFloat(newIndex+3);
                        data.pidFlag.roll.Kp = true;
                        data.newPid.roll = true;
                        break;
                    case MUSV_SERVICE_PID_ROLL_Lp:
                        data.pid.roll.Lp = ByteToFloat(newIndex+3);
                        data.pidFlag.roll.Lp = true;
                        data.newPid.roll = true;
                        break;
                    case MUSV_SERVICE_PID_ROLL_Ki:
                        data.pid.roll.Ki = ByteToFloat(newIndex+3);
                        data.pidFlag.roll.Ki = true;
                        data.newPid.roll = true;
                        break;
                    case MUSV_SERVICE_PID_ROLL_Li:
                        data.pid.roll.Li = ByteToFloat(newIndex+3);
                        data.pidFlag.roll.Li = true;
                        data.newPid.roll = true;
                        break;
                    case MUSV_SERVICE_PID_ROLL_Kd:
                        data.pid.roll.Kd = ByteToFloat(newIndex+3);
                        data.pidFlag.roll.Kd = true;
                        data.newPid.roll = true;
                        break;
                    case MUSV_SERVICE_PID_ROLL_Ld:
                        data.pid.roll.Ld = ByteToFloat(newIndex+3);
                        data.pidFlag.roll.Ld = true;
                        data.newPid.roll = true;
                        break;
                    case MUSV_SERVICE_PID_ROLL_L:
                        data.pid.roll.L = ByteToFloat(newIndex+3);
                        data.pidFlag.roll.L = true;
                        data.newPid.roll = true;
                        break;
                        //Pitch
                    case MUSV_SERVICE_PID_PITCH_Kp:
                        data.pid.pitch.Kp = ByteToFloat(newIndex+3);
                        data.pidFlag.pitch.Kp = true;
                        data.newPid.pitch = true;
                        break;
                    case MUSV_SERVICE_PID_PITCH_Lp:
                        data.pid.pitch.Lp = ByteToFloat(newIndex+3);
                        data.pidFlag.pitch.Lp = true;
                        data.newPid.pitch = true;
                        break;
                    case MUSV_SERVICE_PID_PITCH_Ki:
                        data.pid.pitch.Ki = ByteToFloat(newIndex+3);
                        data.pidFlag.pitch.Ki = true;
                        data.newPid.pitch = true;
                        break;
                    case MUSV_SERVICE_PID_PITCH_Li:
                        data.pid.pitch.Li = ByteToFloat(newIndex+3);
                        data.pidFlag.pitch.Li = true;
                        data.newPid.pitch = true;
                        break;
                    case MUSV_SERVICE_PID_PITCH_Kd:
                        data.pid.pitch.Kd = ByteToFloat(newIndex+3);
                        data.pidFlag.pitch.Kd = true;
                        data.newPid.pitch = true;
                        break;
                    case MUSV_SERVICE_PID_PITCH_Ld:
                        data.pid.pitch.Ld = ByteToFloat(newIndex+3);
                        data.pidFlag.pitch.Ld = true;
                        data.newPid.pitch = true;
                        break;
                    case MUSV_SERVICE_PID_PITCH_L:
                        data.pid.pitch.L = ByteToFloat(newIndex+3);
                        data.pidFlag.pitch.L = true;
                        data.newPid.pitch = true;
                        break;
                        //Yaw
                    case MUSV_SERVICE_PID_YAW_Kp:
                        data.pid.yaw.Kp = ByteToFloat(newIndex+3);
                        data.pidFlag.yaw.Kp = true;
                        data.newPid.yaw = true;
                        break;
                    case MUSV_SERVICE_PID_YAW_Lp:
                        data.pid.yaw.Lp = ByteToFloat(newIndex+3);
                        data.pidFlag.yaw.Lp = true;
                        data.newPid.yaw = true;
                        break;
                    case MUSV_SERVICE_PID_YAW_Ki:
                        data.pid.yaw.Ki = ByteToFloat(newIndex+3);
                        data.pidFlag.yaw.Ki = true;
                        data.newPid.yaw = true;
                        break;
                    case MUSV_SERVICE_PID_YAW_Li:
                        data.pid.yaw.Li = ByteToFloat(newIndex+3);
                        data.pidFlag.yaw.Li = true;
                        data.newPid.yaw = true;
                        break;
                    case MUSV_SERVICE_PID_YAW_Kd:
                        data.pid.yaw.Kd = ByteToFloat(newIndex+3);
                        data.pidFlag.yaw.Kd = true;
                        data.newPid.yaw = true;
                        break;
                    case MUSV_SERVICE_PID_YAW_Ld:
                        data.pid.yaw.Ld = ByteToFloat(newIndex+3);
                        data.pidFlag.yaw.Ld = true;
                        data.newPid.yaw = true;
                        break;
                    case MUSV_SERVICE_PID_YAW_L:
                        data.pid.yaw.L = ByteToFloat(newIndex+3);
                        data.pidFlag.yaw.L = true;
                        data.newPid.yaw = true;
                        break;
                    case MUSV_SERVICE_CALIB_DATA:
                        for (int i = 0; i < 9; i++) {
                            *(data.MTrans+i) = ByteToFloat(newIndex+3+i*4);
                        }
                        for (int i = 0; i < 3; i++) {
                            *(data.MBias+i) = ByteToFloat(newIndex+39+i*4);
                        }
                        data.newCalib = true;
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
                array.remove(0, newIndex + array[newIndex + 1]-1);
                newIndex = 0;
            }
        }
    } while(newIndex >= 0);
}

int protocol::FindNextHead(int startIndex)
{
    int i = startIndex;
    if(i > array.size())
        return -1;
    while(i < array.size())
    {
        switch(array[i])
        {
        case MUSV_IN_ANGLES:
        case MUSV_IN_SERVICE:
        case MUSV_IN_OTHER:
        case MSG_TYPE_OUT_BINS:
        case MUSV_IN_ENCODERS:
        case MUSV_IN_LD_T_D:
            return i;
            break;
        default:
            ++i;
            break;
        }
    }
    return -1;
}

bool protocol::CheckSum(u_int16_t index)
{
    u_int8_t checkSum = 0;
    u_int16_t i;
    for(i = index; i < index+array[index+1]-1; ++i)
    {
        if(i == array.size())
            break;
        checkSum += (u_int8_t) array[i];
    }
    if(checkSum == (u_int8_t)array[i])
    {
        return true;
    }
    return false;
}
