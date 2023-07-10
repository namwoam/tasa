import pandas as pd
import os
import re
import matplotlib.pyplot as plt
import platform

record = []

with open(os.path.join(os.path.dirname(__file__), "latency_record.txt")) as reader:
    lines = reader.readlines()
    for line in lines:
        formatted_line = re.sub("[^a-zA-Z0-9:]+", "", line)
        extracted_data = formatted_line.split(":")
        try:
            record.append(int(extracted_data[2]))
        except IndexError:
            pass
s = pd.Series(record)
df = pd.DataFrame({"latency": s})
df["latency"].plot(kind="hist",bins = 120, logy=True , xlabel="latency (us)" , title=f"{platform.platform()}").get_figure().savefig(
    os.path.join(os.path.dirname(__file__), "hist.png")
)
