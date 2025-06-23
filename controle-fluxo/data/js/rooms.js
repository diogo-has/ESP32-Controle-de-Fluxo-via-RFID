const rooms_div = document.getElementById("rooms-div");

function updateRoomCounts() {
    fetch('../data.json')
        .then(response => response.json())
        .then(data => {
            rooms_div.innerHTML = '';

            data.rooms.forEach(room => {
                const tracker = {};

                room.logs.forEach(log => {
                    const name = log.name;
                    if (name === "Acesso Negado") return;

                    tracker[name] = (tracker[name] || 0) + 1;
                });

                const insideCount = Object.values(tracker).filter(count => count % 2 === 1).length;

                rooms_div.innerHTML += `
                    <div class="entry">
                        <p class="id">Sala ${room.id}</p>
                        <p class="count">Número de Pessoas: ${insideCount}</p>
                    </div>
                `;
            });
        });
}

setInterval(updateRoomCounts, 200);
updateRoomCounts();