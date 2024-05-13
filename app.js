const express = require('express');
const mysql = require('mysql');
const { Client, GatewayIntentBits, EmbedBuilder } = require('discord.js');
const { REST } = require('@discordjs/rest');
const { Routes } = require('discord-api-types/v9');
const os = require('os');
const si = require('systeminformation'); // Import systeminformation module
const ping = require('ping');

// Konfigurasi Discord Bot
const client = new Client({ intents: [GatewayIntentBits.Guilds] });
const channelId = '1239404918385803264';
const warningChannelId = '1239404990565449840';

// Credentials
const TOKEN = 'BOT-TOKEN';
const CLIENT_ID = '1239407606942928946';

const commands = [
    {
        name: 'realtime',
        description: 'CoGarden Realtime',
    },
    {
        name: 'os',
        description: 'Raspberry Pi System Status',
    },
    {
        name: 'recap',
        description: 'CoGarden Date Recap',
        options: [
            {
                name: 'tahun',
                description: 'Tahun',
                type: 4, // Integer
                required: true,
            },
            {
                name: 'bulan',
                description: 'Bulan',
                type: 4, // Integer
                required: false,
            },
            {
                name: 'tanggal',
                description: 'Tanggal',
                type: 4, // Integer
                required: false,
            },
        ],
    },
    {
        name: 'setplant',
        description: 'Set Nama Tanaman',
        options: [
            {
                name: 'nama',
                description: 'Nama Tanaman',
                type: 3, // String
                required: true,
            },
        ],
    },
];

const rest = new REST({ version: '9' }).setToken(TOKEN);

(async () => {
    try {
        console.log('Started refreshing application (/) commands.');

        await rest.put(
            Routes.applicationCommands(CLIENT_ID),
            { body: commands },
        );

        console.log('Successfully reloaded application (/) commands.');
    } catch (error) {
        console.error(error);
    }
})();

// Konfigurasi MySQL
const connection = mysql.createConnection({
    host: '31.6.14.215',
    user: 'tsamalec_cogarden-project',
    password: 'wseecthursina',
    database: 'tsamalec_cogarden'
});

// Konfigurasi Express
const app = express();
const port = 3000;

app.listen(port, () => {
    console.log(`Server berjalan di http://10.10.4.115:${port}`);
});

// Buat tabel MySQL
const createTableQueries = [
    `
    CREATE TABLE IF NOT EXISTS sensor_data (
      id INT AUTO_INCREMENT PRIMARY KEY,
      timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
      humidity FLOAT,
      surrounding_humidity FLOAT,
      temperature FLOAT
    )
  `,
    `
    CREATE TABLE IF NOT EXISTS plant_name (
      id INT AUTO_INCREMENT PRIMARY KEY,
      name VARCHAR(255)
    )
  `
];

for (const query of createTableQueries) {
    connection.query(query, (err) => {
        if (err) throw err;
        console.log('Tabel berhasil dibuat');
    });
}

// Endpoint API
app.get('/cogarden', (req, res) => {
    const { h, sh, t } = req.query;

    // Validasi data
    if (!h || !sh || !t) {
        return res.send('statusgagal');
    }

    // Masukkan data ke MySQL
    const insertQuery = 'INSERT INTO sensor_data (humidity, surrounding_humidity, temperature) VALUES (?, ?, ?)';
    connection.query(insertQuery, [h, sh, t], (err) => {
        if (err) throw err;
        console.log('Data berhasil dimasukkan');
    });

    // Ambil nama tanaman dari database
    const plantNameQuery = 'SELECT name FROM plant_name ORDER BY id DESC LIMIT 1';
    connection.query(plantNameQuery, (err, result) => {
        if (err) throw err;

        const plantName = result.length > 0 ? result[0].name : 'Nama Tanaman belum di Set';

        // Kirim embed ke Discord
        const logEmbed = new EmbedBuilder()
            .setTitle('🌿 CoGarden Logging')
            .setDescription(`**Plant:** ${plantName}`)
            .setColor(0x00AE86)
            .addFields(
                { name: '💧 Humidity', value: h + "%", inline: true },
                { name: '🍃 Surrounding Humidity', value: sh + "%", inline: true },
                { name: '🌡️ Temperature', value: t + "°C", inline: true }
            )
            .setTimestamp();

        const warningCondition = parseFloat(h) < 60 || parseFloat(sh) < 50 || parseFloat(t) < 20;

        if (warningCondition) {
            const warningEmbed = new EmbedBuilder()
                .setTitle('⚠️ CoGarden Notification ⚠️')
                .setColor(0x00AE86)
                .setDescription(`Tanaman ${plantName} Terdeteksi Memiliki ${parseFloat(h) < 60 ? `**Humidity kurang dari 60%! (${h}%)**` : ''} ${parseFloat(sh) < 50 ? `**Surrounding Humidity kurang dari 50%! (${sh}%)**` : ''} ${parseFloat(t) < 20 ? `**Temperature dari 20 Derajat! (${t}°C)**` : ''} Segera Cek Tanaman Kamu.\n\nIni Untuk Detail Tanaman Kamu:`).addFields(
                    { name: '💧 Humidity', value: h + "%", inline: true },
                    { name: '🍃 Surrounding Humidity', value: sh + "%", inline: true },
                    { name: '🌡️ Temperature', value: t + "°C", inline: true }
                )
                .setTimestamp();

            client.channels.cache.get(warningChannelId).send({ embeds: [warningEmbed] });
        } else {
            client.channels.cache.get(channelId).send({ embeds: [logEmbed] });
        }
    });

    res.send('statusoke');
});

// Command /realtime
client.on('interactionCreate', async interaction => {
    if (!interaction.isChatInputCommand()) return;

    if (interaction.commandName === 'realtime') {
        // Ambil data terbaru dari MySQL
        const selectQuery = 'SELECT humidity, surrounding_humidity, temperature, timestamp FROM sensor_data ORDER BY timestamp DESC LIMIT 1';
        connection.query(selectQuery, (err, result) => {
            if (err) throw err;

            if (result.length === 0) {
                interaction.reply('Tidak ada data yang tersedia.');
                return;
            }

            const { humidity, surrounding_humidity, temperature, timestamp } = result[0];

            // Ambil nama tanaman dari database
            const plantNameQuery = 'SELECT name FROM plant_name ORDER BY id DESC LIMIT 1';
            connection.query(plantNameQuery, (err, plantResult) => {
                if (err) throw err;

                const plantName = plantResult.length > 0 ? plantResult[0].name : 'Nama Tanaman belum di Set';

                // Buat embed dengan data terbaru
                const realtimeEmbed = new EmbedBuilder()
                    .setTitle('⏳ CoGarden Realtime')
                    .setColor(0x00AE86)
                    .setDescription(`**Plant:** ${plantName}`)
                    .addFields(
                        { name: '💧 Humidity', value: humidity.toString() + "%", inline: true },
                        { name: '🍃 Surrounding Humidity', value: surrounding_humidity.toString() + "%", inline: true },
                        { name: '🌡️ Temperature', value: temperature.toString() + "°C", inline: true }
                    )
                    .setTimestamp(new Date(timestamp));

                interaction.reply({ embeds: [realtimeEmbed] });
            });
        });
    }
});

// Command /recap
client.on('interactionCreate', async interaction => {
    if (!interaction.isChatInputCommand()) return;

    if (interaction.commandName === 'recap') {
        const year = interaction.options.get('tahun')?.value;
        const month = interaction.options.get('bulan')?.value;
        const day = interaction.options.get('tanggal')?.value;

        if (!year) {
            interaction.reply('Kamu harus memasukkan setidaknya tahun untuk melakukan rekapitulasi.');
            return;
        }

        let dateFilter = '';
        if (day && month) {
            dateFilter = `WHERE DATE(timestamp) = '${year}-${month.toString().padStart(2, '0')}-${day.toString().padStart(2, '0')}'`;
        } else if (month) {
            dateFilter = `WHERE MONTH(timestamp) = ${month} AND YEAR(timestamp) = ${year}`;
        } else {
            dateFilter = `WHERE YEAR(timestamp) = ${year}`;
        }

        const selectQuery = `
            SELECT
              AVG(humidity) AS avg_humidity,
              AVG(surrounding_humidity) AS avg_surrounding_humidity,
              AVG(temperature) AS avg_temperature,
              COUNT(CASE WHEN humidity < 60 OR surrounding_humidity < 50 OR temperature < 20 THEN 1 END) AS warning_count
            FROM sensor_data
            ${dateFilter}
        `;

        connection.query(selectQuery, (err, result) => {
            if (err) throw err;

            const { avg_humidity, avg_surrounding_humidity, avg_temperature, warning_count } = result[0];

            // Ambil nama tanaman dari database
            const plantNameQuery = 'SELECT name FROM plant_name ORDER BY id DESC LIMIT 1';
            connection.query(plantNameQuery, (err, plantResult) => {
                if (err) throw err;

                const plantName = plantResult.length > 0 ? plantResult[0].name : 'Nama Tanaman belum di Set';

                const recapEmbed = new EmbedBuilder()
                    .setTitle('⏳ CoGarden Date Recap')
                    .setColor(0x00AE86)
                    .setDescription(`**Plant:** ${plantName}\n\nIni Adalah rekapitulasi untuk Tanaman Kamu, Semua Data yang ada Dibawah ini Hasil dari Rata-Rata Data.`)
                    .addFields(
                        { name: '💧 Humidity', value: avg_humidity.toFixed(2), inline: true },
                        { name: '🍃 Surrounding Humidity', value: avg_surrounding_humidity.toFixed(2), inline: true },
                        { name: '🌡️ Temperature', value: avg_temperature.toFixed(2), inline: true },
                        { name: '⚠️ Warnings', value: warning_count.toString(), inline: true }
                    );

                interaction.reply({ embeds: [recapEmbed] });
            });
        });
    }
});

// Command /setplant
client.on('interactionCreate', async interaction => {
    if (!interaction.isChatInputCommand()) return;

    if (interaction.commandName === 'setplant') {
        const plantName = interaction.options.get('nama')?.value;

        if (!plantName) {
            interaction.reply('Kamu harus memasukkan nama tanaman.');
            return;
        }

        const insertQuery = 'INSERT INTO plant_name (name) VALUES (?)';
        connection.query(insertQuery, [plantName], (err, result) => {
            if (err) throw err;

            console.log(`Nama tanaman "${plantName}" berhasil diset.`);
            interaction.reply(`Nama tanaman berhasil diset menjadi "${plantName}".`);
        });
    }
});

client.on('interactionCreate', async interaction => {
    if (!interaction.isCommand()) return;
  
    if (interaction.commandName === 'os') {
      const osEmbed = new EmbedBuilder()
        .setTitle('⚙️ CoGarden Monitoring for Raspberry Pi')
        .setColor(0x00AE86)
        .setDescription('System Monitoring for Raspberry Pi System This Script is Used to Check the Health of Raspberry Pi OS System. Look at This Below for Results:')
        .addFields(
            { name: '⏳ Time', value: new Date().toLocaleString(), inline: true },
            { name: '🌡️ Raspberry Pi Temp', value: `${(await si.cpuTemperature()).main} °C`, inline: true },
            { name: '🏬 Storage', value: `${(await si.fsSize()).filter(fs => fs.fs !== 'tmpfs')[0].use}%`, inline: true },
            { name: '📶 Wifi Ping', value: await getPing('8.8.8.8'), inline: true },
            { name: '💻 RAM Usage', value: `${(await si.mem()).active / (await si.mem()).total * 100}%`, inline: true },
            { name: '⚙️ CPU Usage', value: `${(await si.currentLoad()).currentLoad}%`, inline: true }
        )
                interaction.reply({ embeds: [osEmbed] });
    }
  });



  async function getPing(host) {
    try {
      const res = await ping.promise.probe(host);
      return `${res.time} ms`;
    } catch (err) {
      return 'Failed to ping';
    }
  }

// Login bot
client.login(TOKEN);
