#!/Users/immateria/ES-SHELL/es-shell/es

echo "Enhanced ES Shell Syntax Demo"
echo "============================="
echo

# About ES shell pattern matching (~)
echo "=== Understanding ES Shell Pattern Matching ==="
x = 5
name = "Alice"
file = "document.txt"

echo "Pattern matching with ~ operator:"
echo "  {~ \$x 5}      - Does x match '5'? " <={if {~ $x 5} {echo "YES"} {echo "NO"}}
echo "  {~ \$name A*}  - Does name start with 'A'? " <={if {~ $name A*} {echo "YES"} {echo "NO"}}  
echo "  {~ \$file *.txt} - Does file end with '.txt'? " <={if {~ $file *.txt} {echo "YES"} {echo "NO"}}
echo

# Enhanced if-else with parentheses for clarity  
echo "=== Enhanced if-else with parentheses ==="
age = 25

echo "Basic if-else with clearer syntax:"
if-else (~ $age 25) {echo "Age is exactly 25"} {echo "Age is not 25"}

echo "Comparison with traditional ES shell:"
if {~ $age 25} {echo "Traditional ES: Age is 25"}
echo

# Multiple conditions
echo "=== Multiple condition examples ==="
score = 85

echo "Enhanced syntax (much clearer):"
traditional-if (< $score 60) {echo "F - Failing"} \
    elseif (< $score 70) {echo "D - Below Average"} \
    elseif (< $score 80) {echo "C - Average"} \
    elseif (< $score 90) {echo "B - Good"} \
    else {echo "A - Excellent!"}

echo
echo "Traditional ES shell (harder to read):"  
if {< $score 60} {echo "F"} {
    if {< $score 70} {echo "D"} {
        if {< $score 80} {echo "C"} {
            if {< $score 90} {echo "B"} {echo "A"}
        }
    }
}
echo

# List utilities demo
echo "=== Enhanced List Utilities ==="
fruits = (apple banana cherry date elderberry)

echo "Standard ES shell list output:"
echo "  Fruits: $fruits"

echo "Enhanced clean output:"
echo-clean "  Fruits:" $fruits

echo "Joined with custom separators:"
echo "  CSV format: <={join ', ' $fruits}"
echo "  Pipe format: <={join ' | ' $fruits}"
echo

# Pattern matching in practice
echo "=== Pattern Matching in Practice ==="
files = (readme.txt config.json script.sh data.csv backup.tar.gz)

echo "Finding text files:"
for (file = $files) {
    if-else (~ $file *.txt) {echo "  Text file: $file"} {}
}

echo "Finding config files:"  
for (file = $files) {
    if-else (~ $file config.*) {echo "  Config file: $file"} {}
}
echo

# Case statement demo
echo "=== Case Statement Demo ==="
day = Wednesday

echo "Day classification:"
case $day Monday {echo "  Start of the work week"} \
          Tuesday {echo "  Tuesday blues"} \
          Wednesday {echo "  Hump day - halfway there!"} \
          Thursday {echo "  Almost Friday"} \
          Friday {echo "  TGIF!"} \
          {echo "  Weekend time!"}

echo
echo "Demo complete! The enhanced syntax is much more readable."