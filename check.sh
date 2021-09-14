#!/bin/sh

NWORDS=600 # change this to test different values of nWords

echo "Compiling..."
make || exit 1

if [ ! -d "out" ]
then
	echo "Creating out/ directory"
	mkdir "out"
fi

for text in data/*.txt
do
	num=$(basename "$text" .txt)

	echo "$text"

	if [ ! -f "out/$num.$NWORDS.exp" ]
	then
		# expected output
		sed -e '1,/\*\*\* START OF/ d' -e '/\*\*\* END OF/,$ d' < "$text" | tr 'A-Z' 'a-z' | tr -cs "a-z0-9\'-" "\n" | grep '..' | sed -f stop.sed | ./stem | sort | uniq -c | sort -k1nr -k2 | head -n $NWORDS | sed -e 's/ *//' > "out/$num.$NWORDS.exp"
	fi

	# observed output
	./tw $NWORDS "$text" > "out/$num.$NWORDS.out"

	if diff "out/$num.$NWORDS.exp" "out/$num.$NWORDS.out" > /dev/null
	then
		echo "Passed"
	else
		echo "Failed - check differences between out/$num.$NWORDS.exp and out/$num.$NWORDS.out"
	fi
done

