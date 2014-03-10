//
// Yun_WeatherSensors.h
//

// Common structures for communication between the Sensor stations and Base
// station. Sensors communicate with the base via Xbee wireless modules.
// They need a protocol, however, to send commands and results back and
// forth. You could call this "application layer" if you wanted to be
// kind of pedantic about ISO layer naming conventions, I guess.

// These packets fly back and forth as explicit TX send commands and such
// stuffed in the "data" section.


// This could be more efficient - space is used (and transmitted back and
// forth) in requests that is empty. I could make different packet types
// and sizes for requests vs responses, but these packets aren't all that
// big and this would add complexity.

enum PacketType
{
    YWS_requestMeasurement       = 0x1,
    YWS_measurementResponse      = 0x2,
};

enum MeasurementType
{
    YWS_measurement_temp_C        = 0x1,
    YWS_measurement_humidity      = 0x2,
    YWS_measurement_baroPress_kPa = 0x3,
};

enum SensorType
{
    YWS_sensor_MPA115A1 = 0x1,    // Barometric pressure and temp
    YWS_sensor_DHT22    = 0x2,    // Humidity and temperature
    YWS_sensor_TMP36    = 0x3,
};

struct SensorMeasurement
{
    SensorType sensorType;
    MeasurementType measurementType;
    union
    {
        float floatData;
        int intData;
    };
};

struct YWS_XBee_DataPacket
{
    PacketType packetType;

    // For requests, this is ignored.
    SensorMeasurement measurement;
};
