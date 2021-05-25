
# Comment here
const bar = '\n'

"""
    test_fun(a::Int)
For test only
"""
function test_fun(a::Int, b::T) where T <: Number
    println(a)
    println("foo $(bar)")
end

@enum Unicode α=1 β=2

res = [√i for i in 1:10]

#= Dummy function =#
test_fun²(:sym, true, raw"test", `echo 1`)

# function to calculate the volume of a sphere
function sphere_vol(r)
    # julia allows Unicode names (in UTF-8 encoding)
    # so either "pi" or the symbol π can be used
    return 4/3*pi*r^3
end

# functions can also be defined more succinctly
quadratic(a, sqr_term, b) = (-b + sqr_term) / 2a

# calculates x for 0 = a*x^2+b*x+c, arguments types can be defined in function definitions
function quadratic2(a::Float64, b::Float64, c::Float64)
    # unlike other languages 2a is equivalent to 2*a
    # a^2 is used instead of a**2 or pow(a,2)
    sqr_term = sqrt(b^2-4a*c)
    r1 = quadratic(a, sqr_term, b)
    r2 = quadratic(a, -sqr_term, b)
    # multiple values can be returned from a function using tuples
    # if the return keyword is omitted, the last term is returned
    r1, r2
end

vol = sphere_vol(3)
# @printf allows number formatting but does not automatically append the \n to statements, see below
using Printf
@printf "volume = %0.3f\n" vol 
#> volume = 113.097

quad1, quad2 = quadratic2(2.0, -2.0, -12.0)
println("result 1: ", quad1)
#> result 1: 3.0
println("result 2: ", quad2)
#> result 2: -2.0

s1 = "The quick brown fox jumps over the lazy dog α,β,γ"

# search returns the first index of a char
i = findfirst(isequal('b'), s1)
println(i)
#> 11
# the second argument is equivalent to the second argument of split, see below

# or a range if called with another string
r = findfirst("brown", s1)
println(r)
#> 11:15


# string replace is done thus:
r = replace(s1, "brown" => "red")
show(r); println()
#> "The quick red fox jumps over the lazy dog α,β,γ"

# search and replace can also take a regular expressions by preceding the string with 'r'
r = findfirst(r"b[\w]*n", s1)
println(r)
#> 11:15

# again with a regular expression
r = replace(s1, r"b[\w]*n" => "red")
show(r); println()
#> "The quick red fox jumps over the lazy dog α,β,γ"

# there are also functions for regular expressions that return RegexMatch types
# match scans left to right for the first match (specified starting index optional)
r = match(r"b[\w]*n", s1)
println(r)
#> RegexMatch("brown")

# RegexMatch types have a property match that holds the matched string
show(r.match); println()
#> "brown"

# eachmatch returns an iterator over all the matches
r = eachmatch(r"[\w]{4,}", s1)
for i in r print("\"$(i.match)\" ") end
#> "quick" "brown" "jumps" "over" "lazy"
println()


r = collect(m.match for m = eachmatch(r"[\w]{4,}", s1))
println(r)
#> SubString{String}["quick", "brown", "jumps", "over", "lazy"]

# a string can be repeated using the repeat function, 
# or more succinctly with the ^ syntax:
r = "hello "^3
show(r); println() #> "hello hello hello "

# the strip function works the same as python:
# e.g., with one argument it strips the outer whitespace
r = strip("hello ")
show(r); println() #> "hello"
# or with a second argument of an array of chars it strips any of them;
r = strip("hello ", ['h', ' '])
show(r); println() #> "ello"
# (note the array is of chars and not strings)

# similarly split works in basically the same way as python:
r = split("hello, there,bob", ',')
show(r); println() #> SubString{String}["hello", " there", "bob"]
r = split("hello, there,bob", ", ")
show(r); println() #> SubString{String}["hello", "there,bob"]
r = split("hello, there,bob", [',', ' '], limit=0, keepempty=false)
show(r); println() #> SubString{String}["hello", "there", "bob"]
# (the last two arguements are limit and include_empty, see docs)

# the opposite of split: join is simply
r = join(collect(1:10), ", ")
println(r) #> 1, 2, 3, 4, 5, 6, 7, 8, 9, 10

function printsum(a)
    # summary generates a summary of an object
    println(summary(a), ": ", repr(a))
end

# arrays can be initialised directly:
a1 = [1,2,3]
printsum(a1)
#> 3-element Array{Int64,1}: [1, 2, 3]

# or initialised empty:
a2 = []
printsum(a2)
#> 0-element Array{Any,1}: Any[]

# since this array has no type, functions like push! (see below) don't work
# instead arrays can be initialised with a type:
a3 = Int64[]
printsum(a3)
#> 0-element Array{Int64,1}: Int64[]

# ranges are different from arrays:
a4 = 1:20
printsum(a4)
#> 20-element UnitRange{Int64}: 1:20

# however they can be used to create arrays thus:
a4 = collect(1:20)
printsum(a4)
#> 20-element Array{Int64,1}: [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 
#>  15, 16, 17, 18, 19, 20]

# arrays can also be generated from comprehensions:
a5 = [2^i for i = 1:10]
printsum(a5)
#> 10-element Array{Int64,1}: [2, 4, 8, 16, 32, 64, 128, 256, 512, 1024]

# arrays can be any type, so arrays of arrays can be created:
a6 = (Array{Int64, 1})[]
printsum(a6)
#> 0-element Array{Array{Int64,1},1}: Array{Int64,1}[]
# (note this is a "jagged array" (i.e., an array of arrays), not a multidimensional array,
# these are not covered here)

# Julia provided a number of "Dequeue" functions, the most common
# for appending to the end of arrays is push!
# ! at the end of a function name indicates that the first argument is updated.

push!(a1, 4)
printsum(a1)
#> 4-element Array{Int64,1}: [1, 2, 3, 4]

# push!(a2, 1) would cause error:

push!(a3, 1)
printsum(a3) #> 1-element Array{Int64,1}: [1]
#> 1-element Array{Int64,1}: [1]

push!(a6, [1,2,3])
printsum(a6)
#> 1-element Array{Array{Int64,1},1}: Array{Int64,1}[[1, 2, 3]]

# using repeat() to create arrays
# you must use the keywords "inner" and "outer"
# all arguments must be arrays (not ranges)
a7 = repeat(a1,inner=[2],outer=[1])
printsum(a7)
#> 8-element Array{Int64,1}: [1, 1, 2, 2, 3, 3, 4, 4]
a8 = repeat(collect(4:-1:1),inner=[1],outer=[2])
printsum(a8)
#> 8-element Array{Int64,1}: [4, 3, 2, 1, 4, 3, 2, 1]

# try, catch can be used to deal with errors as with many other languages
try
    push!(a,1)
catch err
    showerror(stdout, err, backtrace());println()
end
#> UndefVarError: a not defined
#> Stacktrace:
#>  [1] top-level scope at C:\JuliaByExample\src\error_handling.jl:5
#>  [2] include at .\boot.jl:317 [inlined]
#>  [3] include_relative(::Module, ::String) at .\loading.jl:1038
#>  [4] include(::Module, ::String) at .\sysimg.jl:29
#>  [5] exec_options(::Base.JLOptions) at .\client.jl:229
#>  [6] _start() at .\client.jl:421
println("Continuing after error")
#> Continuing after error

# repeat can be useful to expand a grid
# as in R's expand.grid() function:

m1 = hcat(repeat([1,2],inner=[1],outer=[3*2]),
          repeat([1,2,3],inner=[2],outer=[2]),
          repeat([1,2,3,4],inner=[3],outer=[1]))
printsum(m1)
#> 12×3 Array{Int64,2}: [1 1 1; 2 1 1; 1 2 1; 2 2 2; 1 3 2; 2 3 2; 1 1 3; 2 1 3;
#>   1 2 3; 2 2 4; 1 3 4; 2 3 4]

# for simple repetitions of arrays,
# use repeat
m2 = repeat(m1,1,2)     # replicate a9 once into dim1 and twice into dim2
println("size: ", size(m2))
#> size: (12, 6)

m3 = repeat(m1,2,1)     # replicate a9 twice into dim1 and once into dim2
println("size: ", size(m3))
#> size: (24, 3)

# Julia comprehensions are another way to easily create
# multidimensional arrays

m4 = [i+j+k for i=1:2, j=1:3, k=1:2] # creates a 2x3x2 array of Int64
m5 = ["Hi Im # $(i+2*(j-1 + 3*(k-1)))" for i=1:2, j=1:3, k=1:2]
# expressions are very flexible
# you can specify the type of the array by just
# placing it in front of the expression

import LegacyStrings
m5 = LegacyStrings.ASCIIString["Hi Im element # $(i+2*(j-1 + 3*(k-1)))" for i=1:2, j=1:3, k=1:2]
printsum(m5)
#> 2×3×2 Array{LegacyStrings.ASCIIString,3}: LegacyStrings.ASCIIString[
#>   "Hi Im element # 1" "Hi Im element # 3" "Hi Im element # 5";
#>   "Hi Im element # 2" "Hi Im element # 4" "Hi Im element # 6"]
#>
#> LegacyStrings.ASCIIString["Hi Im element # 7" "Hi Im element # 9"
#>   "Hi Im element # 11"; "Hi Im element # 8" "Hi Im element # 10" "Hi Im element # 12"]

# Array reductions
# many functions in Julia have an array method
# to be applied to specific dimensions of an array:

sum(m4, dims=3)        # takes the sum over the third dimension
sum(m4, dims=(1,3))  # sum over first and third dim

maximum(m4, dims=2)    # find the max elt along dim 2
findmax(m4, dims=3)    # find the max elt and its index along dim 3
                    # (available only in very recent Julia versions)

# Broadcasting
# when you combine arrays of different sizes in an operation,
# an attempt is made to "spread" or "broadcast" the smaller array
# so that the sizes match up. broadcast operators are preceded by a dot:

m4 .+ 3       # add 3 to all elements
m4 .+ [1,2]      # adds vector [1,2] to all elements along first dim

# slices and views
m4=m4[:,:,1] # holds dim 3 fixed
m4[:,2,:]  # that's a 2x1x2 array. not very intuititive to look at

# get rid of dimensions with size 1:
dropdims(m4[:,2,:], dims=2) # that's better

# assign new values to a certain view
m4[:,:,1] = rand(1:6,2,3)
printsum(m4)
#> 2×3 Array{Int64,2}: [3 5 3; 1 3 5]

# (for more examples of try, catch see Error Handling above)
try
    # this will cause an error, you have to assign the correct type
    m4[:,:,1] = rand(2,3)
catch err
    println(err)
end
#> InexactError(:Int64, Int64, 0.7603891754678744)

try
    # this will cause an error, you have to assign the right shape
    m4[:,:,1] = rand(1:6,3,2)
catch err
    println(err)
end
#> DimensionMismatch("tried to assign 3×2 array to 2×3×1 destination")

# dicts can be initialised directly:
a1 = Dict(1=>"one", 2=>"two")
printsum(a1) 
#> Dict{Int64,String} with 2 entries: Dict(2=>"two",1=>"one")

# then added to:
a1[3]="three"
printsum(a1) 
#> Dict{Int64,String} with 3 entries: Dict(2=>"two",3=>"three",1=>"one")
# (note dicts cannot be assumed to keep their original order)

# dicts may also be created with the type explicitly set
a2 = Dict{Int64, AbstractString}()
a2[0]="zero"
printsum(a2)
#> Dict{Int64,AbstractString} with 1 entry: Dict{Int64,AbstractString}(0=>"zero")

# dicts, like arrays, may also be created from comprehensions
using Printf
a3 = Dict([i => @sprintf("%d", i) for i = 1:10])
printsum(a3)
#> Dict{Int64,String} with 10 entries: Dict(7=>"7",4=>"4",9=>"9",10=>"10",
#>  2=>"2",3=>"3",5=>"5",8=>"8",6=>"6",1=>"1")

# as you would expect, Julia comes with all the normal helper functions
# for dicts, e.g., haskey
println(haskey(a1,1)) #> true

# which is equivalent to
println(1 in keys(a1)) #> true
# where keys creates an iterator over the keys of the dictionary

# similar to keys, values get iterators over the dict's values:
printsum(values(a1)) 
#> Base.ValueIterator for a Dict{Int64,String} with 3 entries: ["two", "three", "one"]

# use collect to get an array:
printsum(collect(values(a1)))
#> 3-element Array{String,1}: ["two", "three", "one"]

if true
    println("It's true!")
else
    println("It's false!")
end
#> It's true!

if false
   println("It's true!")
else
   println("It's false!")
end
#> It's false!

# Numbers can be compared with opperators like <, >, ==, !=

1 == 1.
#> true

1 > 2
#> false

"foo" != "bar"
#> true

# and many functions return boolean values

occursin("that", "this and that")
#> true

# More complex logical statments can be achieved with `elseif`

function checktype(x)
   if x isa Int
      println("Look! An Int!")
   elseif x isa AbstractFloat
      println("Look! A Float!")
   elseif x isa Complex
      println("Whoa, that's complex!")
   else
      println("I have no idea what that is")
   end
end

checktype(2)
#> Look! An Int!

checktype(√2)
#> Look! A Float!

checktype(√Complex(-2))
#> Whoa, that's complex!

checktype("who am I?")
#> I have no idea what that is

# For simple logical statements, one can be more terse using the "ternary operator",
# which takes the form `predicate ? do_if_true : do_if_false`

1 > 2 ? println("that's true!") : println("that's false!")
#> that's false!

noisy_sqrt(x) = x ≥ 0 ? sqrt(x) : "That's negative!"
noisy_sqrt(4)
#> 2.0
noisy_sqrt(-4)
#> That's negative!

# "Short-circuit evaluation" offers another option for conditional statements.
# The opperators `&&` for AND and `||` for OR only evaluate the right-hand
# statement if necessary based on the predicate.
# Logically, if I want to know if `42 == 0 AND x < y`,
# it doesn't matter what `x` and `y` are, since the first statement is false.
# This can be exploited to only evaluate a statement if something is true -
# the second statement doesn't even have to be boolean!

everything = 42
everything < 100 && println("that's true!")
#> "that's true!"
everything == 0 && println("that's true!")
#> false

√everything > 0 || println("that's false!")
#> true
√everything == everything || println("that's false!")
#> that's false!

