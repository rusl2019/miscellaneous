import curses
import subprocess


def parse_hosts_file(file_path):
    with open(file_path, "r") as file:
        lines = file.readlines()

    hosts = []
    for line in lines:
        # Skip empty lines and comments
        if not line.strip() or line.startswith("#"):
            continue
        elif line.startswith("127.0.0.1") or line.startswith("::1"):
            continue
        # Split the line into IP and hostnames
        parts = line.split()
        ip = parts[0]
        hostnames = parts[1:]
        # Set default status to 'Unknown'
        hosts.append(
            {
                "ip": ip,
                "hostnames": hostnames,
                "status": "Unknown",
                "cpu": "N/A",
                "mem": "N/A",
            }
        )

    return hosts


def check_host_status(ip):
    try:
        output = subprocess.run(
            ["ping", "-c", "1", "-W", "1", ip],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )
        # Use ping to check the host status
        if output.returncode == 0:
            return "up"
        else:
            return "down"
    except Exception:
        return "unknown"


def get_remote_usage(ip):
    try:
        # Use SSH to run the `top` and `free` commands on the remote host
        cpu_cmd = f"ssh {ip} top -bn1 | grep 'Cpu(s)'"
        mem_cmd = f"ssh {ip} free -m | awk 'NR==2{{printf \"%.2f\", $3*100/$2 }}'"

        cpu_output = subprocess.run(
            cpu_cmd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )
        mem_output = subprocess.run(
            mem_cmd,
            shell=True,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            universal_newlines=True,
        )

        if cpu_output.returncode == 0:
            cpu_usage = cpu_output.stdout.strip().split()[1]
        else:
            cpu_usage = "N/A"

        if mem_output.returncode == 0:
            mem_usage = mem_output.stdout.strip()
        else:
            mem_usage = "N/A"

        return cpu_usage, mem_usage
    except Exception:
        return "N/A", "N/A"


def update_host_statuses(hosts):
    for host in hosts:
        host["status"] = check_host_status(host["ip"])
        if host["status"] == "up":
            host["cpu"], host["mem"] = get_remote_usage(host["ip"])


def display_hosts(stdscr, hosts):
    curses.curs_set(0)  # Hide cursor
    stdscr.clear()
    stdscr.refresh()

    # Define column widths
    ip_col_width = 15
    hostname_col_width = 30
    status_col_width = 10
    cpu_col_width = 10
    mem_col_width = 10

    # Create table header
    header = (
        f"{'IP Address':<{ip_col_width}}"
        + f"{'Hostnames':<{hostname_col_width}}"
        + f" {'Status':<{status_col_width}}"
        + f" {'CPU%':<{cpu_col_width}}"
        + f" {'MEM%':<{mem_col_width}}"
    )
    separator = "-" * (
        ip_col_width
        + hostname_col_width
        + status_col_width
        + cpu_col_width
        + mem_col_width
        + 5
    )

    # Calculate the starting position to center the table
    height, width = stdscr.getmaxyx()
    start_y = (height // 2) - (len(hosts) // 2) - 2
    start_x = (width - len(header)) // 2

    # Display header and separator
    stdscr.addstr(start_y, start_x, header)
    stdscr.addstr(start_y + 1, start_x, separator)

    # Display hosts in the table
    for idx, host in enumerate(hosts, start=start_y + 2):
        stdscr.addstr(idx, start_x, f"{host['ip']:<{ip_col_width}}")
        hostnames_str = " ".join(host["hostnames"])
        stdscr.addstr(
            idx, start_x + ip_col_width + 1, f"{hostnames_str:<{hostname_col_width}}"
        )
        stdscr.addstr(
            idx,
            start_x + ip_col_width + hostname_col_width + 2,
            f"{host['status']:<{status_col_width}}",
        )
        stdscr.addstr(
            idx,
            start_x + ip_col_width + hostname_col_width + status_col_width + 3,
            f"{host['cpu']:<{cpu_col_width}}",
        )
        stdscr.addstr(
            idx,
            start_x
            + ip_col_width
            + hostname_col_width
            + status_col_width
            + cpu_col_width
            + 4,
            f"{host['mem']:<{mem_col_width}}",
        )

    stdscr.refresh()


def main(stdscr):
    file_path = "/etc/hosts"
    hosts = parse_hosts_file(file_path)

    while True:
        update_host_statuses(hosts)
        display_hosts(stdscr, hosts)

        stdscr.nodelay(True)  # Make getch() non-blocking
        key = stdscr.getch()
        if key == ord("q"):  # Press 'q' to exit
            break

        curses.napms(1000)  # Wait for 1 seconds


if __name__ == "__main__":
    curses.wrapper(main)
