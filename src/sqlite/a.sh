DATE="2017-08-22 13:53:00"

for i in {0..8}
do
   NEXT_DATE=$(date +%m-%d-%Y\ %H:%M:%S -d "$DATE + $i day")
   echo "$NEXT_DATE"
done
