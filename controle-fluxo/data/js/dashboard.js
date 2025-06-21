const entry_logs = document.getElementById("entry-logs");

let lastLogCount = 0;

function updateLogs() {
    fetch('../data.json')
        .then(response => response.json())
        .then(data => {
            const logs = data.rooms[0].logs;

            if (logs.length > lastLogCount) {
                entry_logs.innerHTML = '';
                logs.forEach(log => {
                    let i;
                    for (i = 0; i < data.users.length; i++) {
                        if (JSON.stringify(data.users[i].tag) == JSON.stringify(log.tag)) break;
                    }

                    const date = new Date(log.time * 1000);
                    entry_logs.innerHTML += `
                        <div class="entry">
                            <p class="action">Entrou</p>
                            <p class="name">${data.users[i]?.name || "Desconhecido"}</p>
                            <p class="time">${date.toLocaleString("pt-BR")}</p>
                        </div>
                    `;
                });

                lastLogCount = logs.length;
            }
        });
}

setInterval(updateLogs, 1000);

updateLogs();