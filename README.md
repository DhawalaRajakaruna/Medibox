# Medibox Simulations 2 | Embedded Systems Project

This project simulates an IoT-based medicine delivery reminder system that notifies users about medicine times through alarms and a Node-RED dashboard.

## 🛠️ Features

* Alarm-based medicine reminders
* Real-time temperature and humidity monitoring
* OLED display for local status updates
* MQTT communication for efficient data transfer
* Node-RED dashboard integration for remote monitoring

### 📷 Project Snapshots

<table>
  <tr>
    <td align="center">
      <strong>Wokwi Simulation</strong><br>
      <img src="LINK_TO_IMAGE_1" width="300"/>
    </td>
    <td align="center">
      <strong>Node-RED Dashboard</strong><br>
      <img src="LINK_TO_IMAGE_2" width="300"/>
    </td>
  </tr>
</table>


## 🔧 Hardware Used

* ESP32 microcontroller
* DHT11 Temperature & Humidity Sensor
* Servo Motor (for medicine dispensing simulation)
* OLED Display (I2C)

## 🧠 Software & Tools

* Wokwi (for hardware simulation)
* Node-RED (for dashboard interface)
* MQTT protocol (for communication)

## 📡 Communication Flow

1. ESP32 collects environmental data using DHT11.
2. Updates are displayed on OLED.
3. Servo simulates medicine delivery.
4. MQTT sends updates to Node-RED dashboard.
5. User receives alerts at scheduled times.

---
