
## 🌿 CoGarden: A Long-Range Plants/Botanical Monitoring System for Maintaining Stable Health for Plants

## 🧑‍🤝‍🧑 Our Team
- [Rayyan Billal Abdillah](https://www.instagram.com/bix_bbl/)
- [Farrel Naufal Pramono (Me)](https://www.instagram.com/farrelprmn._/)
- [Alif Cryptovan Sinaga](https://www.instagram.com/cryptvn_/)
- [Hibaturahman Althaf Widyadana](https://www.instagram.com/alanisbored/)
- [Atha Haidar Naufal](https://www.instagram.com/_human175_/)

## 🚀 Introduction
**CoGarden** is a Long-Range Monitoring System Using Arduino Uno, ESP32 & Raspberry Pi for the System, Using "Long-Range" or Called [LoRa](https://lora.readthedocs.io/en/latest/). 

## 💻 Arduino Uno Components
- Arduino Uno R3
- 16x Jumper Cable (Female-Male, Male-Male & Female-Female)
- Long Range Module RFM95x
- DHT22 Module for Temperature and Surroundings Humidity
- Soil Moisture Sensor for Soil Humidity
- 16x2 LCD Display for Physical Display

## 🖥️ Raspberry Pi Components
- 20W Type C Power Input
- 16GB SD Card
- 5V DC FAN
- ESP32
- Long Range Module RFM9x

## ⚙️ API Endpoints for Raspberry Pi

#### Input New Data (ESP32)

```http
  GET /cogarden/add
```

| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `h` | `string` | Soil Humidity |
| `sh` | `string` | Surroundings Humidity |
| `t` | `string` | Temperature |

#### Get Realtime Data

```http
  GET /cogarden/realtime
```

#### Get Recap Data

```http
  GET /cogarden/recap
```
| Parameter | Type     | Description                |
| :-------- | :------- | :------------------------- |
| `y` | `string` | Year |
| `m` | `string` | Month |
| `d` | `string` | Date |

## Project Used For

- [WSEEC Competition](https://wsec.or.id/). 


## License

[MIT](https://choosealicense.com/licenses/mit/)






