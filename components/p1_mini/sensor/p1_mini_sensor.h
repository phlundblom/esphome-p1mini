#pragma once

#include <string>

#include "esphome/components/sensor/sensor.h"

#include "../p1_mini.h"

namespace esphome
{
    namespace p1_mini
    {
        class P1MiniSensor : public P1MiniSensorBase, public sensor::Sensor, public Component
        {
        public:
            P1MiniSensor(std::string obis_code)
                : P1MiniSensorBase{ obis_code }
            {}

            virtual void publish_val(double value) override { publish_state(value); }

        };

    } // namespace p1_mini
} // namespace esphome
