import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
from matplotlib.ticker import MaxNLocator
from paint_util import *

def plot_academic_bar(df, x_col, value_cols, 
                      title="performance", x_label="gemm shape", y_label="GFLOPS",
                      figsize=(12, 6), bar_width=0.4, show_grid=True,
                      legend_title="", save_path=None):

    _, ax = plt.subplots(figsize=figsize)
    value_offset = 0.01
    n_bars = len(value_cols)
    group_gap = n_bars * bar_width + 0.2

    x = np.arange(0, len(df[x_col]) * group_gap, group_gap)
    offsets = np.arange(-(len(value_cols)-1)*bar_width/2, 
                       len(value_cols)*bar_width/2, 
                       bar_width)

    for i, col in enumerate(value_cols):
        bars = ax.bar(x + offsets[i], df[col], width=bar_width, label=col, 
               edgecolor='none', linewidth=0.8)
        y_range = ax.get_ylim()[1] - ax.get_ylim()[0]
        offset = y_range * value_offset
        
        for bar in bars:
            height = bar.get_height()
            ax.text(bar.get_x() + bar.get_width()/2., height + offset,
                    "%.2f" % height,
                    ha='center', va='bottom', fontsize=6+(4-n_bars))

    ax.set_title(title, fontsize=14, pad=15)
    ax.set_xlabel(x_label, fontsize=12, labelpad=10)
    ax.set_ylabel(y_label, fontsize=12, labelpad=10)

    ax.set_xticks(x)
    ax.set_xticklabels(df[x_col], fontsize=10)

    if all(df[col].dtype == 'int64' for col in value_cols):
        ax.yaxis.set_major_locator(MaxNLocator(integer=True))

    if show_grid:
        ax.grid(axis='y', linestyle='--', alpha=0.7)

    ax.legend(title=legend_title, fontsize=10, title_fontsize=11, 
              loc='best', frameon=True, framealpha=0.9)

    ax.set_ylim(0, max(df[value_cols].max().values) * 1.2)

    plt.tight_layout()

    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
        print(f"saved: {save_path}")
    plt.close()

def parse_csv(csv_file):
    df = pd.read_csv(csv_file)
    x_col = "shape"
    val_cols = df.columns.tolist()[2:]
    return df, x_col, val_cols

def get_csv_pdf_title_x(arch, shape_type, tn):
    return "../docs/csv/{}/{}_{}.csv".format(arch, shape_type, tn), "../docs/imgs/{}_{}_{}.pdf".format(arch, shape_type, tn), \
        "{}-threaded {} GEMM performance on {}".format(tn, "square" if shape_type == 'MNK' else shape_type, arch), \
        "{} shape".format(shape_type)


def get_title(arch, shape_type, tn):
    return "{}-threaded {} GEMM performance on {}".format(
        tn, "square" if shape_type == 'MNK' else shape_type, arch)

def paint(arch, shape_type, tn):
    csv_file, pdf_path, title, x_label = get_csv_pdf_title_x(arch, shape_type, tn)
    df, x_col, val_cols = parse_csv(csv_file)
    plot_academic_bar(df, x_col, val_cols, title, x_label, "GFLOPS", save_path=pdf_path)


def main():
    pdf_dir = "../docs/imgs"
    if not os.path.exists(pdf_dir):
        os.mkdir(pdf_dir)
    travel_space(paint)

if __name__ == "__main__":
    main()
