for ((i = 10; i <= 1000000; i = i * 10)); do
	dd if=/dev/urandom of=test$i.txt bs=1024 count=$i
done
