import re
import statistics
import matplotlib.pyplot as plt

def parse_metrics(text):
    results = {}
    current_test = None
    for line in text.splitlines():
        m = re.match(r"=== (\w+) test.*===", line)
        if m:
            current_test = m.group(1)
            results[current_test] = []
            continue

        m = re.match(r"pid (\d+): cputime=(\d+), waittime=(\d+)", line)
        if m and current_test:
            pid, cputime, waittime = map(int, m.groups())
            results[current_test].append({
                "pid": pid,
                "cputime": cputime,
                "waittime": waittime
            })
            continue

        if "test done" in line:
            current_test = None

    return results


def load_file(path, label):
    with open(path) as f:
        text = f.read()
    return label, parse_metrics(text)


def analyze(results, label):
    for test, procs in results.items():
        if not procs:
            continue
        cputimes = [p["cputime"] for p in procs]
        waittimes = [p["waittime"] for p in procs]

        print(f"\n[{label}] {test} test")
        print(f"Processes: {len(procs)}")

        print(f" CPU time:"
              f" avg={statistics.mean(cputimes):.2f},"
              f" min={min(cputimes)},"
              f" max={max(cputimes)},"
              f" stdev={statistics.pstdev(cputimes):.2f}")

        print(f" Wait time:"
              f" avg={statistics.mean(waittimes):.2f},"
              f" min={min(waittimes)},"
              f" max={max(waittimes)},"
              f" stdev={statistics.pstdev(waittimes):.2f}")


def plot(all_results, test_name):
    fig, axes = plt.subplots(1, 2, figsize=(12, 5), gridspec_kw={'width_ratios': [1, 2]})
    fig.suptitle(f"{test_name} test", fontsize=14)

    data_wait = [[p["waittime"] for p in results[test_name]] for _, results in all_results]
    labels = [label for label, _ in all_results]

    axes[0].boxplot(data_wait, labels=labels)
    axes[0].set_title("Wait time distribution")
    axes[0].set_ylabel("ticks")

    for label, results in all_results:
        pids = [p["pid"] for p in results[test_name]]
        wait = [p["waittime"] for p in results[test_name]]

        axes[1].bar(pids, wait, alpha=0.6, label=label)

    axes[1].set_title("Wait time per process")
    axes[1].set_ylabel("ticks")
    axes[1].legend()

    plt.tight_layout()
    plt.show()

if __name__ == "__main__":
    files = [
        ("Round Robin", "round_robin.txt"),
        ("MLFQ", "MLFQ.txt"),
    ]

    all_results = [load_file(path, label) for label, path in files]

    # вывод статистики в консоль
    for label, results in all_results:
        analyze(results, label)

    # строим графики для каждого теста
    for test in all_results[0][1].keys():
        plot(all_results, test)
