#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#include "esphome/core/automation.h"

#include <map>

namespace esphome {
    namespace p1_mini {


        class IP1MiniSensor
        {
        public:
            virtual ~IP1MiniSensor() = default;
            virtual void publish_val(double) = 0;
            virtual uint32_t Obis() const = 0;
        };

        class P1MiniSensorBase : public IP1MiniSensor
        {
            uint32_t const m_obis;
        public:
            P1MiniSensorBase(std::string obis_code);
            virtual uint32_t Obis() const { return m_obis; }
        };

        class ReadyToReceiveTrigger : public Trigger<> { };
        class UpdateReceivedTrigger : public Trigger<> { };
        class CommunicationErrorTrigger : public Trigger<> { };


        class P1Mini : public uart::UARTDevice, public Component {
        public:
            P1Mini(uint32_t min_period_ms, bool secondary_p1);

            void setup() override;
            void loop() override;
            void dump_config() override;

            void register_sensor(IP1MiniSensor *sensor)
            {
                m_sensors.emplace(sensor->Obis(), sensor);
            }

            void register_ready_to_receive_trigger(ReadyToReceiveTrigger *trigger) { m_ready_to_receive_triggers.push_back(trigger); }
            void register_update_received_trigger(UpdateReceivedTrigger *trigger) { m_update_received_triggers.push_back(trigger); }
            void register_communication_error_trigger(CommunicationErrorTrigger *trigger) { m_communication_error_triggers.push_back(trigger); }

        private:

            unsigned long m_identifying_message_time{ 0 };
            unsigned long m_reading_message_time{ 0 };
            unsigned long m_verifying_crc_time{ 0 };
            unsigned long m_processing_time{ 0 };
            unsigned long m_waiting_time{ 0 };
            unsigned long m_error_recovery_time{ 0 };
            int m_num_message_loops{ 0 };
            int m_num_processing_loops{ 0 };
            bool m_display_time_stats{ false };
            uint32_t obis_code{ 0x00 };

            // Store the message as it is being received:
            constexpr static int message_buffer_size{ 2048 };
            char m_message_buffer[message_buffer_size];
            int m_message_buffer_position{ 0 };
            int m_crc_position{ 0 };

            // Keeps track of the start of the data record while processing.
            char *m_start_of_data;

            char GetByte()
            {
                char C{ static_cast<char>(read()) };
                if (m_secondary_p1) write(C);
                return C;
            }

            enum class states {
                IDENTIFYING_MESSAGE,
                READING_MESSAGE,
                VERIFYING_CRC,
                PROCESSING_ASCII,
                PROCESSING_BINARY,
                WAITING,
                ERROR_RECOVERY
            };
            enum states m_state { states::ERROR_RECOVERY };

            void ChangeState(enum states new_state);

            enum class data_formats {
                UNKNOWN,
                ASCII,
                BINARY
            };
            enum data_formats m_data_format { data_formats::UNKNOWN };

            uint32_t const m_min_period_ms;
            bool const m_secondary_p1;

            std::map<uint32_t, IP1MiniSensor *> m_sensors;
            std::vector<ReadyToReceiveTrigger *> m_ready_to_receive_triggers;
            std::vector<UpdateReceivedTrigger *> m_update_received_triggers;
            std::vector<CommunicationErrorTrigger *> m_communication_error_triggers;

            constexpr static int discard_log_num_bytes{ 32 };
            char m_discard_log_buffer[discard_log_num_bytes * 2 + 1];
            char *m_discard_log_position{ m_discard_log_buffer };
            char *const m_discard_log_end{ m_discard_log_buffer + (discard_log_num_bytes * 2) };

            void AddByteToDiscardLog(uint8_t byte);
            void FlushDiscardLog();

        };


    }  // namespace p1_mini_component
}  // namespace esphome
