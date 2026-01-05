
export function renderLinkQualityChart(data) {
    const lq_params = ["SNR", "PER", "PHY"];
    const leftGridContainer = document.getElementById('LeftGridContainer');

    for (let i = 0; i < data.Devices.length; i++) {
        const dev = data.Devices[i];
        const chart_data = [];

        const chartId = `chart-${i}`;
        let div = document.getElementById(chartId);

        if (!div) {
            // Create wrapper only once
            const wrapper = document.createElement('div');
            wrapper.style.background = '#fff';
            wrapper.style.padding = '5px';

            div = document.createElement('div');
            div.id = chartId;
            div.style.height = '400px';

            wrapper.appendChild(div);
            leftGridContainer.appendChild(wrapper);
        }

        // Add link quality lines
        for (const key of lq_params) {
            chart_data.push({
                x: dev.Time,
                y: dev.LinkQuality[key],
                type: 'scatter',
                mode: 'lines',
                name: key
            });
        }

        // Add Score line
        chart_data.push({
            x: dev.Time,
            y: dev.LinkQuality.Score,
            type: 'scatter',
            mode: 'lines',
            name: 'Score',
            line: { width: 2 }
        });

        // Add Alarms as red dots
        const alarmX = [];
        const alarmY = [];
        for (let j = 0; j < dev.LinkQuality.Alarms.length; j++) {
            if (dev.LinkQuality.Alarms[j]) {
                alarmX.push(dev.Time[j]);
                alarmY.push(dev.LinkQuality.Score[j]);
            }
        }

        chart_data.push({
            x: alarmX,
            y: alarmY,
            type: 'scatter',
            mode: 'markers',
            name: 'Alarm',
            marker: { color: 'red', size: 10 }
        });

        // Layout
        const layout = {
            title: { text: `Link Quality: ${dev.MAC}`, pad: { t: 25 } },
            margin: { t: 70, l: 45, r: 20, b: 80 },
            xaxis: {
                title: { text: 'Time' },
                type: 'category',
                tickmode: 'array',
                tickvals: dev.Time,
                ticktext: dev.Time,
                tickangle: -45,
                automargin: true
            },
            yaxis: {
                title: { text: 'Units' },
                range: [0, 1],
                tick0: 0,
                dtick: 0.1
            }
        };

        // Update existing chart or create new chart
        Plotly.react(div, chart_data, layout, { responsive: true });
    }
}

function updateAlarmDot(show) {
    const dot = document.getElementById("alarmDot");
    dot.style.display = show ? "inline-block" : "none";
}

export function renderAlarms(data) {
    const table = document.getElementById("Alarms");

    // Reset table header
    table.innerHTML = `
      <tr>
        <th>Device</th>
        <th>Description</th>
        <th>Time</th>
      </tr>`;

    let hasAlarm = false;

    for (const dev of data.Devices) {
        const alarms = dev.LinkQuality?.Alarms || [];

        for (let i = 0; i < alarms.length; i++) {
            if (!alarms[i]) continue;

            hasAlarm = true;

            const row = document.createElement("tr");
            row.innerHTML = `
              <td>${dev.MAC}</td>
              <td>Link Quality Alarm</td>
              <td>${alarms[i]}</td>`;
            table.appendChild(row);
        }
    }

    updateAlarmDot(hasAlarm);
}

