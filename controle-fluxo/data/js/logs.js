const entry_logs = document.getElementById("entry-logs");

let lastLogCount = 0;

function updateLogs() {
    fetch('../data.json')
        .then(response => response.json())
        .then(data => {
            const logs = data.rooms[0].logs;

            let tracker = {};
            if (logs.length > lastLogCount) {
                entry_logs.innerHTML = '';
                logs.forEach(log => {
                    let tagstr = ''
                    log.tag.forEach(byte => {tagstr += byte.toString(16).toUpperCase().padStart(2, '0') + ' '})

                    if (tracker[tagstr] == undefined) tracker[tagstr] = 0;

                    // let i;
                    // for (i = 0; i < data.users.length; i++) {
                    //     if (JSON.stringify(data.users[i].tag) == JSON.stringify(log.tag)) break;
                    // }

                    tracker[tagstr]++

                    const date = new Date(log.time * 1000);
                    entry_logs.innerHTML += `
                        <div class="entry">
                            <p class="action">${log.name == "Acesso Negado" ? "Acesso Negado" : (tracker[tagstr] % 2 == 0 ? "Saiu" : "Entrou")}</p>
                            <p class="name">${log.name == "Acesso Negado" ? tagstr : log.name}</p>
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