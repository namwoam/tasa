echo "running io critical test"
timeout -k 10s 10s sudo ./io_critical
echo "running computation critical test"
timeout -k 10s 10s sudo ./compute_critical