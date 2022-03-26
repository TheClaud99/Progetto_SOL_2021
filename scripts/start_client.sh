breakLoop=$((SECONDS+30))
echo "$1"
index=1

# itero per 30 secondi
while [ $SECONDS -le $breakLoop ]; do

	./client -f ./sock_file.sk \
		-d tests/outputs/readed \
		-D tests/outputs/deleted \
		-W "tests/a/d/test - Copia (${index}).txt" \
		-w "tests/a/d/" 3 \
		-r "tests/a/d/test - Copia (${index}).txt" \
		-R 3 \
		-c "$PWD/tests/a/d/test - Copia (${index}).txt" >/dev/null 2>/dev/null
	index=$((index + 1))

	if [ $((index % 20)) = 1 ]; then
		index=1
	fi
done

exit 0
