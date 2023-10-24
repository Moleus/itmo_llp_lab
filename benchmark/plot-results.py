# read benchmark.csv and plot the results
# file contains header: insert_time,delete_time,mem_usage,file_size
# 0.002040,0.000000,0,1028
# 0.003659,0.000000,0,1028

import matplotlib.pyplot as plt
import pandas as pd
import os

file_path = "../cmake-build-release-system/benchmark/benchmark.csv"


def plot_results():
    # read csv file
    df = pd.read_csv(file_path)
    columns = list(df.columns[:])
    for i, column in enumerate(columns):
        plt.figure()
        plt.xlabel("batches")
        plt.ylabel(column)
        # range from 1 to columns.len
        plt.plot(df["row"], df[column])
        # plt.show()
        name = column.lower().split(" ")[0] + ".png"
        if not os.path.exists("lab_bench_res"):
            os.mkdir("lab_bench_res")
        plt.savefig("lab_bench_res/" + name)


if __name__ == "__main__":
    plot_results()
