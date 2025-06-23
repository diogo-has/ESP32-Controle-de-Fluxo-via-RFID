const check = document.getElementById("check");
const name_field = document.getElementById("name");
const perms_field = document.getElementById("perms");

function confirmNewTag() {
    let name = name_field.value;
    let perm = perms_field.value;

    fetch(`/add_user?name=${encodeURIComponent(name)}&perm=${encodeURIComponent(perm)}`)
        .then(response => {
            if (response.ok) {
                window.location.href = "/dashboard-users";
            } else {
                alert("Erro ao adicionar usuário.");
            }
        })
}

fetch("/start_newtag");
var interval = setInterval(() => {
    fetch('/check_newtag')
        .then(response => response.text())
        .then(data => {
            if (data === "success") {
                check.style.visibility = "hidden";
                clearInterval(interval);
            } else if (data === "return") {
                window.location.href = "/dashboard-users"
            }
        });
}, 200);

