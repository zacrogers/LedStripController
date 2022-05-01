#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

constexpr uint8_t pwm_max         = UINT8_MAX;
constexpr uint8_t num_channels    = 4;

// Index offsets for the state display led pins
constexpr uint8_t pwr_leds_offset = 0;
constexpr uint8_t sel_leds_offset = 4;

// Pin assignments
// constexpr uint8_t disp_led_pins[num_channels * 2] = {
//     GPIO_NUM_0, GPIO_NUM_0, GPIO_NUM_0, GPIO_NUM_0,   // Channel power leds
//     GPIO_NUM_0, GPIO_NUM_0, GPIO_NUM_0, GPIO_NUM_0    // Currently selected channel leds
// }; // apply offsets when using in loops

constexpr uint8_t disp_led_pins[num_channels * 2] = {0};

constexpr uint8_t channel_pins[num_channels] = { 0, 0, 0, 0 };
constexpr uint8_t status_led_pin             = 0;

constexpr uint8_t encoder_pins[]             = { 0, 0 };
constexpr uint8_t encoder_btn_pin            = 0;

// State enums
enum class Mode
{
    BINARY,
    PWM
};

enum class Press
{
    SHORT,
    LONG,
    NONE
};

Mode    mode                           = Mode::BINARY;
Press   button_last_press              = Press::NONE;

uint8_t channel_pwm_val[num_channels]  = { 0, 0, 0, 0 };
bool    channel_enabled[num_channels]  = { false, false, false, false };

uint8_t encoder_prev_val               = 0;
uint8_t encoder_curr_val               = 0;
bool    encoder_val_changed            = false;

uint8_t selected_channel               = 0;



void check_encoder_button();
void read_encoder();
void binary_mode();
void pwm_mode();
void set_outputs();

void setup() 
{
    for(uint8_t i = 0; i < (num_channels * 2); ++i)
    {
        if(i < num_channels)
        {
            pinMode(channel_pins[i], OUTPUT);
        }

        // pinMode(disp_led_pins[i], OUTPUT);
    }

    pinMode(status_led_pin, OUTPUT);

    pinMode(encoder_btn_pin, INPUT);
    pinMode(encoder_pins[0], INPUT);
    pinMode(encoder_pins[1], INPUT);
}


void loop() 
{
    // Check the inputs
    check_encoder_button();
    read_encoder();

    // Do the thing
    switch (mode)
    {
        case Mode::BINARY: 
            binary_mode();

        case Mode::PWM:    
            pwm_mode();
    }

    // Set the outputs
    if((button_last_press == Press::SHORT) || (button_last_press == Press::LONG) || encoder_val_changed)
    {
        set_outputs();
    }

    // Clear all flags
    button_last_press    = Press::NONE;
    encoder_val_changed  = false;
}


void binary_mode()
{
    if(button_last_press == Press::LONG)
    {
        mode = Mode::PWM;
    }
    else if(button_last_press == Press::SHORT)
    {
        channel_enabled[selected_channel] = !channel_enabled[selected_channel];
    }
}


void pwm_mode()
{
    if(button_last_press == Press::SHORT)
    {
        mode = Mode::BINARY;
    }
}

void set_outputs()
{
    for(uint8_t i = 0; i < num_channels; ++i)
    {
        // Set channel select leds
        if(selected_channel == i)
        {
            digitalWrite(disp_led_pins[i + sel_leds_offset], HIGH);
        }
        else
        {
            digitalWrite(disp_led_pins[i + sel_leds_offset], LOW);
        }

        // Set power status leds and channel outputs
        if(channel_enabled[i])
        {
            //analogWrite(disp_led_pins[i + pwr_leds_offset], channel_pwm_val[i]);
            //analogWrite(channel_pins[i + pwr_leds_offset], channel_pwm_val[i]);
        }
        else
        {
            //analogWrite(disp_led_pins[i + pwr_leds_offset], 0);
            //analogWrite(channel_pins[i + pwr_leds_offset], 0);
        }
        
    }
}


void check_encoder_button()
{
    if(digitalRead(encoder_btn_pin))
    {
        button_last_press = Press::SHORT;
    }
    else
    {
        button_last_press = Press::NONE;
    }
}

void read_encoder()
{
    if(encoder_prev_val != encoder_curr_val)
    {
        encoder_val_changed = true;
    }

    encoder_prev_val = encoder_curr_val;
}


void channel_write_pwm(uint8_t channel, uint8_t pwm)
{
    if(pwm < 0)
    {
        pwm = 0;
    }
    else if (pwm > pwm_max)
    {
        pwm = pwm_max;
    }

    if(0 == pwm)
    {
        digitalWrite(disp_led_pins[channel] + pwr_leds_offset, LOW);
    }
    else
    {
        digitalWrite(disp_led_pins[channel] + pwr_leds_offset, HIGH);
    }

}
