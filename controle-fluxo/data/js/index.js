const info = document.getElementById("info");
const check = document.getElementById("check");

function adminCheck() {
    info.style.opacity = "0%"
    info.style.visibility = "hidden"
    check.style.opacity = "100%"
    check.style.visibility = "visible"
    fetch("/start_auth");
    setInterval(() => {
        fetch('/check_redirect')
            .then(response => response.text())
            .then(data => {
                if (data === "redirect") {
                    window.location.href = "/dashboard";  // redireciona
                } else if (data === "reload") {
                    location.reload();
                }
            });
    }, 200);
}