#include "AC.h"

#include <EEPROM.h>
#include <IRRemoteControl.h>
#include <IRRemoteControlInt.h>
#include "SmartRoomConfiguration.h"
#include "external_configuration.h"

void IR_ISR();

IRSend irsend;

// IR Variant Buffer(Received from Server or hardcoded for standalone mode)
volatile uint16_t ir_variants[] = {EXTERNAL_VARIATIONS};

// Threshold for variants(also received from server)
volatile uint16_t ir_variant_thresholds[] = {EXTERNAL_THRESHOLDS};

// Storage for the server-defined ir-pattern
volatile uint8_t ir_pattern_data[] = {EXTERNAL_IR_PATTERN};

// Storage for the 4-bit encoded ir_values
volatile uint8_t ir_values[IR_BUFFER_LENGTH] = {};
//////////////////////////////

//////////////////////////////
// State Machine vars
//////////////////////////////
// Index of current captured value(used for both pre-matching and actual storage)
volatile uint16_t ir_value_index = 0;

// Keep track of last IR change's timestamp
volatile unsigned long last_micros;

// How many variants were sent from server
volatile uint8_t ir_variant_count = sizeof(ir_variants) / sizeof(uint16_t);

volatile uint8_t ir_pattern_length = sizeof(ir_pattern_data);

// Is the FSM discarding samples?
volatile bool discarding_ir_samples = false;

// Does the data match the pattern?
volatile bool pattern_matched = false;

// Current micros() value(Used on ISR mainly, so no volatile needed)
volatile unsigned long current_micros = 0;

// Register sample into buffer(automatically selects the buffer)
int8_t register_ir_sample();

// Copy the matching buffer into the real buffer
void copy_pre_buffer();
//////////////////////////////

//////////////////////////////
// Global Variables
//////////////////////////////
CompressedIRRecv irRecv;
//////////////////////////////

AC::AC()
{
    pinMode(SmartRoomConfiguration::ir_recv_pin, INPUT);
    pinMode(SmartRoomConfiguration::ir_send_pin, OUTPUT);
}

void AC::initialize()
{
    this->last_user_ir_length;

    resume();
}

#define BLINK100 \
    {            \
    } //\
    //{                                    \
        //digitalWrite(LED_BUILTIN, HIGH); \
        //delay(200);                      \
        //digitalWrite(LED_BUILTIN, LOW);  \
        //delay(200);                      \
    //}

IREventType AC::task()
{
    // Use a scaler to lower the decode attempt rate(It was too high, less than 30us between iterations)
    uint64_t current_millis = millis();
    static uint64_t last_iteration = 0;

    if ((current_millis - last_iteration > 10) && irRecv.decode())
    {
        last_iteration = current_millis;
        BLINK100

        if (ir_value_index > SmartRoomConfiguration::minimum_ac_ir_train_length)
        {
            BLINK100

            this->last_user_ir_length = ir_value_index;
            this->resume();

            return IREventAC;
        }
        else
        {
            this->resume();
            return IREventOther;
        }
    }

    return IREventNone;
}

bool current_ac_status = false;

void AC::set_power(bool power_state)
{
    if (current_ac_status == power_state)
        return;
    current_ac_status = power_state;

    if (power_state)
    {
        this->send_compressed_train(const_cast<uint8_t *>(ir_values), this->last_user_ir_length);
    }
    else
    {
        this->send_compressed_train((uint8_t *)this->ir_idle_values, this->ir_idle_length);
    }
}

void AC::resume()
{
    irRecv.resume();
}

void AC::send_compressed_train(uint8_t *data, uint16_t length)
{
    irRecv.stop();
    irsend.enableIROut(38);

    for (uint16_t i = 0; i < length; i++)
    {
        uint32_t decompressedValue = get_compressed_value(ir_variants, data, i);
        if (i & 1)
        {
            irsend.space(decompressedValue);
        }
        else
        {
            irsend.mark(decompressedValue);
        }
    }

    irsend.space(0);
    irRecv.resume();
}

const int8_t NO_VARIANT_MATCHES = -1;
const int8_t SKIPPED_FIRST_SAMPLE = -2;

#define match_variant(length, index) \
    ((length >= ir_variants[index] - ir_variant_thresholds[index]) && (length <= ir_variants[index] + ir_variant_thresholds[index]))

int8_t get_variant_from_sample(unsigned long sample)
{
    for (int variant_index = 0; variant_index < ir_variant_count; variant_index++)
    {
        if (match_variant(sample, variant_index))
        {
            return (int8_t)variant_index;
        }
    }

    return NO_VARIANT_MATCHES;
}

int8_t register_ir_sample()
{
    // Current delta
    unsigned long delta = 0;

    current_micros = micros();
    delta = current_micros - last_micros;
    last_micros = current_micros;

    int8_t variant_id = get_variant_from_sample(delta);

    if (delta < UINT16_MAX && variant_id >= 0)
    {
        bool odd = ir_value_index & 0x1;

        uint16_t buffer_index = ir_value_index / 2;

        if (ir_value_index >= ir_pattern_length)
        {
            if (odd)
                ir_values[buffer_index] = (variant_id) | (ir_values[buffer_index] & 0xF0);
            else
                ir_values[buffer_index] = (((uint8_t)variant_id) << 4) | (ir_values[buffer_index] & 0x0F);
        }

        ir_value_index++;
        return variant_id;
    }
    else
    {
        if (ir_value_index == 0)
        {
            return SKIPPED_FIRST_SAMPLE;
        }
    }

    return NO_VARIANT_MATCHES;
}

void IR_ISR()
{
    // digitalWrite(13, HIGH);
    if (last_micros == 0)
    {
        last_micros = micros();
        // digitalWrite(13, LOW);
        return;
    }

    // if (discarding_ir_samples)
    // {
    //     last_micros = micros();
    // digitalWrite(13, LOW);
    //     return;
    // }

    if (pattern_matched)
    {
        register_ir_sample();

        if (ir_value_index >= IR_BUFFER_LENGTH)
        {
            detachInterrupt(digitalPinToInterrupt(SmartRoomConfiguration::ir_recv_pin));
            discarding_ir_samples = true;
            return;
        }
    }
    else
    {
        int8_t variant = register_ir_sample();

        if (variant == SKIPPED_FIRST_SAMPLE)
        {
            return;
        }

        if (variant != ir_pattern_data[ir_value_index - 1])
        {
            // Pattern mismatch!
            ir_value_index = 0;

            if (variant == ir_pattern_data[0])
            {
                ir_value_index++;
            }

            discarding_ir_samples = true;
            return;
        }

        if (ir_value_index == ir_pattern_length)
        {
            // Ok, pattern matches!
            copy_pre_buffer();
            pattern_matched = true;
        }
    }

    digitalWrite(13, LOW);
}

void copy_pre_buffer()
{
    for (int i = 0; i < ir_pattern_length; i++)
    {
        if (i & 0b1)
            ir_values[i / 2] = (ir_pattern_data[i]) | (ir_values[i / 2] & 0xF0);
        else
            ir_values[i / 2] = (((uint8_t)ir_pattern_data[i]) << 4) | (ir_values[i / 2] & 0x0F);
    }
}

void CompressedIRRecv::resume()
{
    // debugln(F("ResumeIR"))
    discarding_ir_samples = false;
    pattern_matched = false;
    last_micros = 0;
    current_micros = 0;
    ir_value_index = 0;

    attachInterrupt(digitalPinToInterrupt(SmartRoomConfiguration::ir_recv_pin), IR_ISR, CHANGE);
}

void CompressedIRRecv::stop()
{
    detachInterrupt(digitalPinToInterrupt(SmartRoomConfiguration::ir_recv_pin));
}

bool CompressedIRRecv::decode()
{
    auto time_delta = micros() - last_micros;

    if ((ir_value_index > 0) && (time_delta > (ir_variants[(sizeof(ir_variants) / sizeof(uint16_t)) - 1] + 2000)))
    {
        last_micros = 0;
        stop();

        return true;
    }

    return false;
}
