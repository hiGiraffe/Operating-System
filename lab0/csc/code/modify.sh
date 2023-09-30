file=$1
str1=$2
str2=$3

sed -i "s/$str1/$str2/g" "$file" 
