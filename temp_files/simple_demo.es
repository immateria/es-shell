#!/Users/immateria/ES-SHELL/es-shell/es

echo "ES Shell Enhanced Syntax - Simple Demo"
echo "======================================"
echo

# Show the difference between ~ pattern matching and equality
echo "=== ES Shell Pattern Matching (~) vs Equality ==="
x = 5
echo "Variable x = $x"
echo

# Basic pattern matching examples
echo "Pattern matching examples:"
echo "  ~ \$x 5      --> " <={if {~ $x 5} {echo "matches"} {echo "no match"}}
echo "  ~ \$x [0-9]  --> " <={if {~ $x [0-9]} {echo "matches"} {echo "no match"}}
echo "  ~ \$x 1*     --> " <={if {~ $x 1*} {echo "matches"} {echo "no match"}}
echo

# File pattern examples
filename = "document.txt"
echo "File pattern matching with '$filename':"
echo "  ~ \$filename *.txt  --> " <={if {~ $filename *.txt} {echo "matches"} {echo "no match"}}  
echo "  ~ \$filename *.pdf  --> " <={if {~ $filename *.pdf} {echo "matches"} {echo "no match"}}
echo

# Show list improvements  
echo "=== Enhanced List Handling ==="
colors = (red green blue yellow)

echo "Standard ES list output:"
echo "  Colors: $colors"
echo

echo "With join function:"
echo "  Comma separated: <={join \", \" $colors}"
echo "  Pipe separated: <={join \" | \" $colors}"
echo

# Simple case statement
echo "=== Case Statement Demo ==="
fruit = apple

echo "Classifying fruit '$fruit':"
case $fruit apple {echo "  It's an apple!"} \
           banana {echo "  It's a banana!"} \
           orange {echo "  It's an orange!"} \
           {echo "  Unknown fruit"}
echo

# Traditional if still works
echo "=== Traditional ES Shell Syntax Still Works ==="
if {~ $x 5} {echo "Traditional if: x equals 5"}
echo

echo "Demo complete!"