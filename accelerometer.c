/*
 * This program is used to get shift from accelerometer data.
 * Author: Richard Lin
 * Date:    2016/09/07
*/

#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include "math.h"
#include <stdio.h>

#define COUNT_OF_ARRAY( _Array )           ( sizeof( _Array ) / sizeof( ( _Array )[0] ) )

#define DEBUG

FILE* g_pFile = NULL;

#ifdef DEBUG
static inline void _TRACE_LOW(const char *fileName, int lineNo, const char *format, va_list* args )
{
    char buf[1024];
    char finalFormat[300];
    struct timeval timeVal;
    gettimeofday( &timeVal, NULL );
    snprintf( finalFormat,
              COUNT_OF_ARRAY( finalFormat ),
              "[%ld.%06ld]:[Trace][%s:%d]%s\n",
              timeVal.tv_sec,
              timeVal.tv_usec,
              fileName,
              lineNo,
              format );
    finalFormat[ COUNT_OF_ARRAY( finalFormat ) - 1 ] = '\0';
    if( vsnprintf( buf, COUNT_OF_ARRAY( buf ), finalFormat, *args ) == -1 )
        strncpy( buf, "vsnprintf return failed! can not output log.", (COUNT_OF_ARRAY( buf )) );
    buf[ COUNT_OF_ARRAY( buf ) - 1 ] = '\0';
    printf( "%s", buf );
    fflush( stdout );
}
void TraceLow( const char *fileName, int lineNo, const char *format, ... )
{
    va_list args;
    va_start( args, format );
    _TRACE_LOW( fileName, lineNo, format, &args );
    va_end( args );
}
#define TRACE( _Format, ... )   TraceLow( __FILE__, __LINE__, _Format, ##__VA_ARGS__ )
#else
#define TRACE( _Format, ... )
#endif

//#define META_DATA_FILE_PATH "./in.txt"
#define META_DATA_FILE_PATH "sensortext.cvs"

/*
* Macro Definition & Global variable
*/
#define INTEGRAL_FUNCTION
#define ZEROED_CALIBRATION
#define LOW_PASS_FILTER
#define WHITE_NOISE_FILTER
#define DATA_READER
#define DATA_OUTPUT
#define DRAW_FUNCTION

typedef long double META_DATA_TYPE;

typedef struct
{
    META_DATA_TYPE x;
    META_DATA_TYPE y;
    META_DATA_TYPE z;
}META_DATA_AXES_TYPE;

typedef META_DATA_AXES_TYPE ACCELEROMETER_META_DATA_TYPE;
typedef META_DATA_AXES_TYPE VELOCITY_TYPE;
typedef META_DATA_AXES_TYPE SHIFT_TYPE;

#define ZERO_REFERENCE_COUNT 1024
ACCELEROMETER_META_DATA_TYPE zero_array[ZERO_REFERENCE_COUNT];

#define AVERAGE_SAMPLE_COUNT 64
ACCELEROMETER_META_DATA_TYPE average_array[AVERAGE_SAMPLE_COUNT];

#define INTEGRAL_ARRAY_SIZE 1024
ACCELEROMETER_META_DATA_TYPE Integral_Array[INTEGRAL_ARRAY_SIZE];

#define READ_BUF_SIZE (AVERAGE_SAMPLE_COUNT*INTEGRAL_ARRAY_SIZE)
#define VALID_DATA_BUF_SIZE READ_BUF_SIZE
#define LOW_PASS_BUF_SIZE READ_BUF_SIZE
ACCELEROMETER_META_DATA_TYPE read_buffer[READ_BUF_SIZE];
ACCELEROMETER_META_DATA_TYPE valid_data_buffer[VALID_DATA_BUF_SIZE];
ACCELEROMETER_META_DATA_TYPE low_pass_buffer[LOW_PASS_BUF_SIZE];
long int current_valid_data_length;
long int current_read_buffer_length;
long int current_low_pass_length;

#define VELOCITY_BUFFER_SIZE READ_BUF_SIZE
VELOCITY_TYPE velocity_buffer[VELOCITY_BUFFER_SIZE];
long int current_velocity_buffer_length;

#define SHIFT_BUFFER_SIZE READ_BUF_SIZE
SHIFT_TYPE shift_buffer[SHIFT_BUFFER_SIZE];
long int current_shift_buffer_length;

#define CAPTURE_INTERVAL 0.000001 /* 1 micro second*/
#define INTEGRAL_SAMPLE_COUNT 1024

/*
* Integral Function
*/
#ifdef INTEGRAL_FUNCTION
SHIFT_TYPE integral_function(long int sample_count, ACCELEROMETER_META_DATA_TYPE* sample_array)
{
    
}
#endif/*INTEGRAL_FUNCTION*/

/*
* Get zero reference
*/
void get_zero_reference(ACCELEROMETER_META_DATA_TYPE* mean_value)
{
    int i;
    long double x_t = 0;
    long double y_t = 0;
    long double z_t = 0;
    for(i=0;i<ZERO_REFERENCE_COUNT;i++)
    {
        x_t += zero_array[i].x;
        y_t += zero_array[i].y;
        z_t += zero_array[i].z;
    }
    mean_value ->x = x_t/ZERO_REFERENCE_COUNT;
    mean_value ->y = y_t/ZERO_REFERENCE_COUNT;
    mean_value ->z = z_t/ZERO_REFERENCE_COUNT;
    TRACE("%.7Lf, %.7Lf, %.7Lf",mean_value ->x,mean_value ->y,mean_value ->z);
}

/*
* Zeroed Calibration
*/
#ifdef ZEROED_CALIBRATION
void zeroed_calibration(\
ACCELEROMETER_META_DATA_TYPE upper, \
ACCELEROMETER_META_DATA_TYPE lowwer, \
long int sample_count, \
ACCELEROMETER_META_DATA_TYPE*input_array, \
ACCELEROMETER_META_DATA_TYPE**output_array)
{
}
#endif/*ZEROED_CALIBRATION*/

/*
* Low Pass Filter
*/
#ifdef LOW_PASS_FILTER
void low_pass_calibration(\
long int sample_count, \
ACCELEROMETER_META_DATA_TYPE* input_array, \
ACCELEROMETER_META_DATA_TYPE**output_array)
{
}
#endif/*LOW_PASS_FILTER*/

/*
* White Noise Filter
*/
#ifdef WHITE_NOISE_FILTER
int white_noise_calibration(\
long int sample_count, \
ACCELEROMETER_META_DATA_TYPE* input_array, \
ACCELEROMETER_META_DATA_TYPE* *output_array)
{
}
#endif/*WHITE_NOISE_FILTER*/

/*
* Data Reader
*/
#ifdef DATA_READER

int string_to_long_double(char* str, int str_len, long double * val)
{
    //-009.7005005
    int i = 0;
    int decimal_length = 0;
    char* pBuf = 0;
    char* pBuftail = 0;
    char is_minus = 0;
    long int integer_part = 0;
    long int fraction_part = 0;
    char* current_ptr = str;
    char* decimal_point_ptr = NULL;
    int string_length = str_len;

    if(0 >= string_length)
    {
        *val = 0.0L;
        TRACE("string length error!");
        return -1;
    }
    
    if('-' == str[0])
    {
        is_minus = 1;
        current_ptr +=1;
        string_length -=1;
    }

    for(i=0;i<string_length;i++)
        if('.' == *(current_ptr+i))
            decimal_point_ptr = current_ptr+i;
    if(NULL == decimal_point_ptr)
        decimal_point_ptr = current_ptr + string_length;

    for(pBuf = current_ptr; pBuf < decimal_point_ptr; pBuf++)
    {
        integer_part *= 10;
        integer_part += (*pBuf - '0');
    }
    pBuftail = (current_ptr+string_length);
    for(pBuf = decimal_point_ptr + 1; pBuf < pBuftail;pBuf++)
    {
        fraction_part *= 10;
        fraction_part += (*pBuf - '0');
    }
    decimal_length = pBuftail - decimal_point_ptr - 1;
    if(decimal_length < 0)
        decimal_length = 0;
    *val = (long double)integer_part + ((long double)fraction_part/(long double)pow10(decimal_length));
    if(1 == is_minus)
        *val = 0 - (*val);

    return 0;
    
}

int read_one(FILE* pFile, long double* x_out, long double* y_out, long double* z_out)
{
    char file_buff[1024];
    int i;
    int j;
    int line_length;
    char* x_ptr = 0;
    char* y_ptr = 0;
    char* z_ptr = 0;
    char* line_end_ptr = 0;

    long double x_value;
    long double y_value;
    long double z_value;
    
    if(NULL== pFile)
    {
        TRACE("fail to open %s",META_DATA_FILE_PATH);
    }
    else
    {
        while(fgets(file_buff,1024,pFile)!=NULL)
        {
            x_ptr = 0;
            y_ptr = 0;
            z_ptr = 0;
            if('A' == file_buff[0])
            {
                line_length = strlen(file_buff);
                j = 0;
                for(i=0;i<line_length;i++)
                {
                    if(',' == file_buff[i])
                    {
                        j++;
                        if(3 == j)
                        {
                            int k;
                            x_ptr = file_buff + i + 2;
                            for(k=i+2;k<line_length;k++)
                            {
                                if(',' == file_buff[k])
                                {
                                    int n;
                                    y_ptr = file_buff + k + 2;
                                    for(n=k+2;n<line_length;n++)
                                    {
                                        if(',' == file_buff[n])
                                        {
                                            z_ptr = file_buff + n + 2;
                                            for(n+=2;n<line_length;n++)
                                                if( ('.' != file_buff[n]) && ( ('0'>file_buff[n])||('9'<file_buff[n]) ) )
                                                    line_end_ptr = file_buff + n;
                                            goto OutCal;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            OutCal:
            if((0 != x_ptr)&&(0 != y_ptr)&&(0 != z_ptr))
            {
                string_to_long_double(x_ptr,(y_ptr-x_ptr-2),&x_value);
                string_to_long_double(y_ptr,(z_ptr-y_ptr-2),&y_value);
                string_to_long_double(z_ptr,(line_end_ptr-z_ptr),&z_value);
                //TRACE("00%.7Lf, 00%.7Lf, %.7Lf",x_value,y_value,z_value);
                *x_out = x_value;
                *y_out = y_value;
                *z_out = z_value;
                return 0;
            }
            
        }
    }
    return 1;
}

long int data_reader(\
long int read_count, \
ACCELEROMETER_META_DATA_TYPE* *output_array, \
long int* output_count)
{
}

#define X_ZERO_ABSOLUTE 0.017
#define Y_ZERO_ABSOLUTE 0.017
#define Z_ZERO_ABSOLUTE 0.017
#define MIN_COUNT_OF_ZERO_POINT 8

char data_read_zero_array(long int read_count,long int* output_count)
{
    long int i=0;
    long int j=0;
    long double x_1;
    long double y_1;
    long double z_1;
    long double x_2;
    long double y_2;
    long double z_2;
    ACCELEROMETER_META_DATA_TYPE* pArray = zero_array;
    while(1)
    {
        if(0 == read_one(g_pFile,&x_1,&y_1,&z_1))
        {
            if(0 == i)
            {
                if(0 == read_one(g_pFile,&x_2,&y_2,&z_2))
                {
                    if((fabs(x_2-x_1)<X_ZERO_ABSOLUTE)&&(fabs(y_2-y_1)<X_ZERO_ABSOLUTE)&&(fabs(z_2-z_1)<X_ZERO_ABSOLUTE))
                    {
                        TRACE("add 1st sample");
                        pArray[i].x = x_2;
                        pArray[i].y = y_2;
                        pArray[i].z = z_2;
                        i++;
                        if(i == read_count)
                            return 0;
                    }
                    else
                    {
                        TRACE("drop 2 sample");
                    }
                }
                else
                    return 1;
            }
            else
            {
                if((fabs(pArray[i-1].x-x_1)<X_ZERO_ABSOLUTE)\
                    &&(fabs(pArray[i-1].y-y_1)<X_ZERO_ABSOLUTE)\
                    &&(fabs(pArray[i-1].z-z_1)<X_ZERO_ABSOLUTE))
                {
                        TRACE("add 1 sample after %d",i);
                        pArray[i].x = x_1;
                        pArray[i].y = y_1;
                        pArray[i].z = z_1;
                        i++;
                        if(i == read_count)
                            return 0;
                }
                else
                {
                    TRACE("drop %d sample",i%MIN_COUNT_OF_ZERO_POINT+1);
                    i = i - i%MIN_COUNT_OF_ZERO_POINT;
                }
            }
        }
        else
        {
            return 1;
        }
    }
}
#endif/*DATA_READER*/

/*
* Data Output
*/
#ifdef DATA_OUTPUT
void shift_data_output(SHIFT_TYPE s)
{
}
#endif/*DATA_OUTPUT*/

/*
* Draw Function
*/
#ifdef DRAW_FUNCTION
void draw_shift(long int shift_array_length, SHIFT_TYPE* shift_array)
{
}
#endif/*DRAW_FUNCTION*/

/*
* Initialize Array
*/
void initialize_array(long int count, META_DATA_AXES_TYPE* array)
{
}

int main(int argc, char** argv)
{
    long int i;
    long int result = 0;
    long int read_count = 0;
    SHIFT_TYPE position;

    ACCELEROMETER_META_DATA_TYPE upper_ref;
    ACCELEROMETER_META_DATA_TYPE lowwer_ref;
    ACCELEROMETER_META_DATA_TYPE mean_ref;

    g_pFile = fopen(META_DATA_FILE_PATH,"r");

    if(NULL== g_pFile)
    {
        TRACE("fail to open %s",META_DATA_FILE_PATH);
        return 0;
    }
    
    initialize_array(ZERO_REFERENCE_COUNT, zero_array);
    initialize_array(AVERAGE_SAMPLE_COUNT, average_array);
    initialize_array(INTEGRAL_ARRAY_SIZE, Integral_Array);
    initialize_array(READ_BUF_SIZE, read_buffer);
    initialize_array(LOW_PASS_BUF_SIZE, low_pass_buffer);
    initialize_array(VALID_DATA_BUF_SIZE, valid_data_buffer);

    initialize_array(VELOCITY_BUFFER_SIZE, velocity_buffer);
    initialize_array(SHIFT_BUFFER_SIZE, shift_buffer);
    
    current_valid_data_length = 0;
    current_read_buffer_length = 0;
    current_low_pass_length = 0;
    current_velocity_buffer_length = 0;
    current_shift_buffer_length = 0;
    
    data_read_zero_array(ZERO_REFERENCE_COUNT, &read_count);

    get_zero_reference(&mean_ref);

    return 0;

    /*

    while(1)
    {
        result = data_reader(READ_BUF_SIZE,&read_buffer, &read_count);
        if( (-1 == result) || (READ_BUF_SIZE != read_count) )
            break;
        zeroed_calibration(upper_ref,lowwer_ref,read_count,&read_buffer);
        result = white_noise_calibration(read_count, read_buffer, &low_pass_buffer);
        if(LOW_PASS_BUF_SIZE == current_low_pass_length)
        {
            low_pass_calibration(current_low_pass_length,low_pass_buffer,&valid_data_buffer);
            current_low_pass_length = 0;
            if(VALID_DATA_BUF_SIZE == current_valid_data_length)
            {
                for(i=0;i<current_valid_data_length;i+=INTEGRAL_SAMPLE_COUNT)
                {
                    position = integral_function(INTEGRAL_SAMPLE_COUNT,(valid_data_buffer+i));
                    shift_data_output(position);
                    draw_shift(current_shift_buffer_length,shift_buffer);
                }
                current_valid_data_length = 0;
            }
        }
    }
    return 0;*/
}
