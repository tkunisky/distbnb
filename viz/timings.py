import argparse
import collections
import glob

from matplotlib import pyplot as plt
import numpy as np

from viz import output_util


parser = argparse.ArgumentParser()
parser.add_argument('--input_pattern', type=str, required=True)
parser.add_argument('--output_file', type=str, default=None)
args = parser.parse_args()


def main():
    seriess = collections.defaultdict(lambda: collections.defaultdict(list))

    for filename in glob.glob(args.input_pattern):
        ret = output_util.read_output_file(filename)
        if 'WORKERS' in ret and 'SIZE' in ret and 'TIME' in ret:
            seriess[ret['WORKERS']][ret['SIZE']].append(ret['TIME'])

    colors = ['red', 'green', 'blue']
    for i, (num_workers, series) in enumerate(
            sorted(seriess.items(), key=lambda t: t[0])):
        keys = sorted(series.keys())
        mean_times = [np.mean(series[k]) for k in keys]
        all_times = sum([series[k] for k in keys], [])
        all_times_keys = sum([len(series[k]) * [k] for k in keys], [])
        label = '%d Worker' % num_workers + ('s' if num_workers > 1 else '')

        plt.scatter(
            keys,
            mean_times,
            label=label,
            s=20,
            c=colors[i],
            zorder=10 * (i + 1))
        plt.scatter(
            all_times_keys,
            all_times,
            alpha=0.2,
            s=5,
            c=colors[i],
            zorder=1 * (i + 1))

    ax = plt.gca()
    ax.set_yscale('log')
    ax.grid(alpha=0.3)
    ax.set_axisbelow(True)
    ax.minorticks_on()
    ax.set_xlabel('Problem Size (Graph Vertices)')
    ax.set_ylabel('Time (Seconds)')
    plt.legend()
    plt.tight_layout()

    if args.output_file is None:
        plt.show()
    else:
        plt.savefig(args.output_file)


if __name__ == '__main__':
    main()
