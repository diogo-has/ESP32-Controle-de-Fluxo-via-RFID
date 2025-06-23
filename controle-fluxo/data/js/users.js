const users_div = document.getElementById("users-div");

fetch('../data.json')
    .then(response => response.json())
    .then(data => {
        data.users.forEach(user => {
            let tag = ''
            user.tag.forEach(byte => {tag += byte.toString(16).toUpperCase().padStart(2, '0') + ' '})

            let clearance = ''
            user.clearance.forEach(room => {clearance += room + ' '})

            users_div.innerHTML += `
            <div class="entry">
                <p class="nome">${user.name}</p>
                <p class="tag">${tag}</p>
                <p class="clearance">${clearance}</p>
                <p class="perms">${user.permissions}</p>
            </div>
            `;
        });
    });

function deleteUser() {
    let id = prompt("Deletar usuário de qual posição?", 2);

    fetch(`/delete_user?id=${encodeURIComponent(id)}`)
        .then(response => {
            if (response.ok) {
                location.reload();
            } else {
                alert("Erro ao deletar usuário.");
            }
        })
}