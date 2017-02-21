/*
  Our test script
 */
//rm switchtest.txt;./randomn 500 | sed 's/^/add /g' | tee -a switchtest.txt | sed 's/[^0-9 ]//g; s/ /\n/g'| sed '/^$/d;s/^/del /g'  >> switchtest.txt
//sed -n '1p'  switchtest.txt > rest.txt; echo "print" >> rest.txt | ./myB-TreeTest < rest.txt > unsorted; sort -n unsorted > sorted; diff sorted unsorted

