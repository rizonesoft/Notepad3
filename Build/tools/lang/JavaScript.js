// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Lexical_grammar
// https://www.ecma-international.org/publications/standards/Ecma-262.htm

async await
break
case catch class const continue
debugger default delete do
else export extends
finally for function
if import in instanceof
new
return
super switch
this throw try typeof
var void
while with
yield
let
static
enum
implements interface package private protected public

Infinity
NaN
undefined

// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-imports
// https://www.ecma-international.org/ecma-262/9.0/index.html#sec-exports
import as from;
export as;
export default;

eval(x)
isFinite(number)
isNaN(number)
parseFloat(string)
parseInt(string, radix)
decodeURI(encodedURI)
decodeURIComponent(encodedURIComponent)
encodeURI(uri)
encodeURIComponent(uriComponent)
escape(string)
unescape(string)

Object([value]) {
	assign(target, ...sources)
	create(O, Properties)
	defineProperties(O, Properties)
	defineProperty(O, P, Attributes)
	entries(O)
	freeze(O)
	getOwnPropertyDescriptor(O, P)
	getOwnPropertyDescriptors(O)
	getOwnPropertyNames(O)
	getOwnPropertySymbols(O)
	getPrototypeOf(O)
	is(value1, value2)
	isExtensible(O)
	isFrozen(O)
	isSealed(O)
	keys(O)
	preventExtensions(O)
	seal(O)
	setPrototypeOf(O, proto)
	values(O)
	prototype
		constructor
		hasOwnProperty(V)
		isPrototypeOf(V)
		propertyIsEnumerable(V)
		toLocaleString([reserved1 [, reserved2]])
		toString()
		valueOf()

		__proto__
		__defineGetter__(P, getter)
		__defineSetter__(P, setter)
		__lookupGetter__(P)
		__lookupSetter__(P)
}

Function(p1, p2, … , pn, body) {
	length
	name
	prototype
		apply(thisArg, argArray)
		bind(thisArg, ...args)
		call(thisArg, ...args)
		toString()
}

Boolean(value)

Symbol([description]) {
	asyncIterator
	hasInstance
	isConcatSpreadable
	iterator
	match
	replace
	search
	species
	split
	toPrimitive
	toStringTag
	unscopables
	prototype
	for(key)
	keyFor(sym)
}

Error(message) {
	message
	name

	EvalError
	RangeError
	ReferenceError
	SyntaxError
	TypeError
	URIError
	NativeError
}

Number(value) {
	EPSILON
	MAX_SAFE_INTEGER
	MAX_VALUE
	MIN_SAFE_INTEGER
	MIN_VALUE
	NaN
	NEGATIVE_INFINITY
	POSITIVE_INFINITY

	isFinite(number)
	isInteger(number)
	isNaN(number)
	isSafeInteger(number)
	parseFloat(string)
	parseInt(string, radix)
	prototype
		toExponential(fractionDigits)
		toFixed(fractionDigits)
		toPrecision(precision)
}

Math {
	E
	LN10
	LN2
	LOG10E
	LOG2E
	PI
	SQRT1_2
	SQRT2
	toStringTag

	abs(x)
	acos(x)
	acosh(x)
	asin(x)
	asinh(x)
	atan(x)
	atanh(x)
	atan2(y, x)
	cbrt(x)
	ceil(x)
	clz32(x)
	cos(x)
	cosh(x)
	exp(x)
	expm1(x)
	floor(x)
	fround(x)
	hypot(value1, value2, ...values)
	imul(x, y)
	log(x)
	log1p(x)
	log10(x)
	log2(x)
	max(value1, value2, ...values)
	min(value1, value2, ...values)
	pow(base, exponent)
	random()
	round(x)
	sin(x)
	sinh(x)
	sqrt(x)
	tan(x)
	tanh(x)
	trunc(x)
}

Date(year, month [, date [, hours [, minutes [, seconds [, ms]]]]]) {
	Date(value)
	Date()
	UTC(year [, month [, date [, hours [, minutes [, seconds [, ms]]]]]])
	now()
	parse(string)
	prototype
		getDate()
		getDay()
		getFullYear()
		getHours()
		getMilliseconds()
		getMinutes()
		getMonth()
		getSeconds()
		getTime()
		getTimezoneOffset()
		getUTCDate()
		getUTCDay()
		getUTCFullYear()
		getUTCHours()
		getUTCMilliseconds()
		getUTCMinutes()
		getUTCMonth()
		getUTCSeconds()
		setDate(date)
		setFullYear(year [, month [, date]])
		setHours(hour [, min [, sec [, ms]]])
		setMilliseconds(ms)
		setMinutes(min [, sec [, ms]])
		setMonth(month [, date])
		setSeconds(sec [, ms])
		setTime(time)
		setUTCDate(date)
		setUTCFullYear(year [, month [, date]])
		setUTCHours(hour [, min [, sec [, ms]]])
		setUTCMilliseconds(ms)
		setUTCMinutes(min [, sec [, ms]])
		setUTCMonth(month [, date])
		setUTCSeconds(sec [, ms])
		toDateString()
		toISOString()
		toJSON(key)
		toLocaleDateString([reserved1 [, reserved2]])
		toLocaleTimeString([reserved1 [, reserved2]])
		toTimeString()
		toUTCString()

		getYear()
		setYear(year)
		toGMTString()
}

String(value) {
	fromCharCode(...codeUnits)
	fromCodePoint(...codePoints)
	raw(template, ...substitutions)
	length
	prototype
		charAt(pos)
		charCodeAt(pos)
		codePointAt(pos)
		concat(...args)
		endsWith(searchString [, endPosition])
		includes(searchString [, position])
		indexOf(searchString [, position])
		lastIndexOf(searchString [, position])
		localeCompare(that [, reserved1 [, reserved2]])
		match(regexp)
		normalize([form])
		padEnd(maxLength [, fillString])
		padStart(maxLength [, fillString])
		repeat(count)
		replace(searchValue, replaceValue)
		search(regexp)
		slice(start, end)
		split(separator, limit)
		startsWith(searchString [, position])
		substring(start, end)
		toLocaleLowerCase([reserved1 [, reserved2]])
		toLocaleUpperCase([reserved1 [, reserved2]])
		toLowerCase()
		toUpperCase()
		trim()
		iterator
			next()

		substr(start, length)
		anchor(name)
		big()
		blink()
		bold()
		fixed()
		fontcolor(color)
		fontsize(size)
		italics()
		link(url)
		small()
		strike()
		sub()
		sup()

}

RegExp(pattern, flags) {
	compile(pattern, flags)
	lastIndex
	prototype
		exec(string)
		test(S)
		dotAll
		flags
		global
		ignoreCase
		multiline
		sticky
		source
		unicode
}

Array(...items) {
	Array(len)
	Array()
	from(items [, mapfn [, thisArg]])
	isArray(arg)
	of(...items)
	length
	prototype
		concat(...arguments)
		copyWithin(target, start [, end])
		entries()
		every(callbackfn [, thisArg])
		fill(value [, start [, end]])
		filter(callbackfn [, thisArg])
		find(predicate [, thisArg])
		findIndex(predicate [, thisArg])
		forEach(callbackfn [, thisArg])
		includes(searchElement [, fromIndex])
		indexOf(searchElement [, fromIndex])
		join(separator)
		keys()
		lastIndexOf(searchElement [, fromIndex])
		map(callbackfn [, thisArg])
		pop()
		push(...items)
		reduce(callbackfn [, initialValue])
		reduceRight(callbackfn [, initialValue])
		reverse()
		shift()
		slice(start, end)
		some(callbackfn [, thisArg])
		sort(comparefn)
		splice(start, deleteCount, ...items)
		unshift(...items)
		values()
		iterator
		unscopables

	TypedArray
		Int8Array
		Uint8Array
		Uint8ClampedArray
		Int16Array
		Uint16Array
		Int32Array
		Uint32Array
		Float32Array
		Float64Array
}

Map([iterable]) {
	prototype
		clear()
		delete(key)
		entries()
		forEach(callbackfn [, thisArg])
		get(key)
		has(key)
		keys()
		set(key, value)
		values()
		size
		iterator
		toStringTag
}

Set([iterable]) {
	prototype
		add(value)
		clear()
		delete(value)
		entries()
		forEach(callbackfn [, thisArg])
		has(value)
		keys()
		values()
		size
}

WeakMap([iterable]) {
	prototype
		delete(key)
		get(key)
		has(key)
		set(key, value)
}

WeakSet([iterable]) {
	prototype
		add(value)
		delete(value)
		has(value)
}

ArrayBuffer(length) {
	isView(arg)
	prototype
		byteLength
		slice(start, end)
}

SharedArrayBuffer(length) {
	prototype
		byteLength
		slice(start, end)
}

DataView(buffer [, byteOffset [, byteLength]]) {
	prototype
		buffer
		byteLength
		byteOffset
		getFloat32(byteOffset [, littleEndian])
		getFloat64(byteOffset [, littleEndian])
		getInt8(byteOffset)
		getInt16(byteOffset [, littleEndian])
		getInt16(byteOffset [, littleEndian])
		getInt32(byteOffset [, littleEndian])
		getUint8(byteOffset)
		getUint16(byteOffset [, littleEndian])
		getUint32(byteOffset [, littleEndian])
		setFloat32(byteOffset, value [, littleEndian])
		setFloat64(byteOffset, value [, littleEndian])
		setInt8(byteOffset, value)
		setInt16(byteOffset, value [, littleEndian])
		setInt32(byteOffset, value [, littleEndian])
		setUint8(byteOffset, value)
		setUint16(byteOffset, value [, littleEndian])
		setUint32(byteOffset, value [, littleEndian])
}

Atomics {
	add(typedArray, index, value)
	and(typedArray, index, value)
	compareExchange(typedArray, index, expectedValue, replacementValue)
	exchange(typedArray, index, value)
	isLockFree(size)
	load(typedArray, index)
	or(typedArray, index, value)
	store(typedArray, index, value)
	sub(typedArray, index, value)
	wait(typedArray, index, value, timeout)
	wake(typedArray, index, count)
	xor(typedArray, index, value)
}

JSON {
	parse(text [, reviver])
	stringify(value [, replacer [, space]])
}

Generator {
	prototype
		next(value)
		return(value)
		throw(exception)
}

Promise(executor) {
	all(iterable)
	race(iterable)
	reject(r)
	resolve(x)
	prototype
		catch(onRejected)
		then(onFulfilled, onRejected)
}

AsyncFunction(p1, p2, … , pn, body) {
}

Reflect {
	apply(target, thisArgument, argumentsList)
	construct(target, argumentsList [, newTarget])
	defineProperty(target, propertyKey, attributes)
	deleteProperty(target, propertyKey)
	get(target, propertyKey [, receiver])
	getOwnPropertyDescriptor(target, propertyKey)
	getPrototypeOf(target)
	has(target, propertyKey)
	isExtensible(target)
	ownKeys(target)
	preventExtensions(target)
	set(target, propertyKey, V [, receiver])
	setPrototypeOf(target, proto)
}

Proxy(target, handler) {
	revocable(target, handler)
}

// https://developer.mozilla.org/en-US/docs/Web/API/XMLHttpRequest
// https://xhr.spec.whatwg.org/
XMLHttpRequest() {
	UNSENT
	OPENED
	HEADERS_RECEIVED
	LOADING
	DONE

	onreadystatechange
	readyState
	response
	responseText
	responseType
	responseURL
	responseXML
	status
	statusText
	timeout
	upload
	withCredentials
	abort()
	getAllResponseHeaders()
	getResponseHeader(headerName)
	open(method, url [, async [, user[, password]]])
	overrideMimeType(mimeType)
	send(body)
	setRequestHeader(header, value)
}

// https://developer.mozilla.org/en-US/docs/Web/API/FormData
// https://xhr.spec.whatwg.org/#formdata
FormData([form]) {
	append(name, value [, filename])
	delete(name)
	delete(name)
	get(name)
	getAll(name)
	has(name)
	keys()
	set(name, value [, filename])
	values()
}

// https://developer.mozilla.org/en-US/docs/Web/API/WindowOrWorkerGlobalScope
WindowOrWorkerGlobalScope {
	atob(encodedData)
	btoa(stringToEncode)
	clearInterval(intervalID)
	clearTimeout(timeoutID)
	createImageBitmap(image[, sx, sy, sw, sh[, options]]).then(function(response) { ... })
	Promise<Response> fetch(input[, init])
	setInterval(func, delay[, param1, param2, ...])
	setTimeout(function[, delay, param1, param2, ...])
}

// https://developer.mozilla.org/en-US/docs/Web/API/Storage
Storage {
	length
	clear()
	getItem()
	key()
	removeItem()
	setItem()
	localStorage
	sessionStorage
}

// https://nodejs.org/api/globals.html
__dirname
__filename
exports
module
require(path)
