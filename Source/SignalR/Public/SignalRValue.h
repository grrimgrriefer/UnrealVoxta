/*
 * MIT License
 *
 * Copyright (c) 2020-2022 Frozen Storm Interactive, Yoann Potinet
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include "CoreMinimal.h"

 /**
  * Represents a value that can be sent to or received from a SignalR hub.
  * Supports various data types including numbers, objects, arrays, strings, booleans, and binary data.
  */
class FSignalRValue
{
public:
	/**
	 * Enumeration of possible value types that can be stored in a FSignalRValue.
	 */
	enum class EValueType
	{
		Number,
		Object,
		Array,
		String,
		Null,
		Boolean,
		Binary
	};

	/**
	 * Create an object representing a EValueType::Null value.
	 */
	FSignalRValue() :
		Type(EValueType::Null)
	{
		Value.Emplace<NumberType>(0);
	}

	/**
	 * Create an object representing a EValueType::Null value.
	 */
	FSignalRValue(std::nullptr_t) :
		Type(EValueType::Null)
	{
		Value.Emplace<NumberType>(0);
	}

	/**
	 * Create an object representing a EValueType::Number with the given integer value.
	 * 
	 * @param InValue The integer value to store.
	 */
	FSignalRValue(const int32 InValue) :
		Type(EValueType::Number)
	{
		Value.Set<NumberType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Number with the given unsigned integer value.
	 * 
	 * @param InValue The unsigned integer value to store.
	 */
	FSignalRValue(const uint32 InValue) :
		Type(EValueType::Number)
	{
		Value.Set<NumberType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Number with the given 64-bit integer value.
	 * 
	 * @param InValue The 64-bit integer value to store.
	 */
	FSignalRValue(const int64 InValue) :
		Type(EValueType::Number)
	{
		Value.Set<NumberType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Number with the given unsigned 64-bit integer value.
	 * 
	 * @param InValue The unsigned 64-bit integer value to store.
	 */
	FSignalRValue(const uint64 InValue) :
		Type(EValueType::Number)
	{
		Value.Set<NumberType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Number with the given float value.
	 * 
	 * @param InValue The float value to store.
	 */
	FSignalRValue(const float InValue) :
		Type(EValueType::Number)
	{
		Value.Set<NumberType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Number with the given double value.
	 * 
	 * @param InValue The double value to store.
	 */
	FSignalRValue(const double InValue) :
		Type(EValueType::Number)
	{
		Value.Set<NumberType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Object with the given map of string-values.
	 * 
	 * @param InValue The map of key-value pairs to store.
	 */
	FSignalRValue(const TMap<FString, FSignalRValue>& InValue) :
		Type(EValueType::Object)
	{
		Value.Set<TSharedPtr<ObjectType>>(MakeShared<ObjectType>(InValue));
	}

	/**
	 * Create an object representing a EValueType::Object with the given map of string-values.
	 * 
	 * @param InValue The map of key-value pairs to store (moved).
	 */
	FSignalRValue(TMap<FString, FSignalRValue>&& InValue) :
		Type(EValueType::Object)
	{
		Value.Emplace<TSharedPtr<ObjectType>>(MakeShared<ObjectType>(MoveTemp(InValue)));
	}

	/**
	 * Create an object representing a EValueType::Array with the given array of values.
	 * 
	 * @param InValue The array of values to store.
	 */
	FSignalRValue(const TArray<FSignalRValue>& InValue) :
		Type(EValueType::Array)
	{
		Value.Set<ArrayType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Array with the given array of values.
	 * 
	 * @param InValue The array of values to store (moved).
	 */
	FSignalRValue(TArray<FSignalRValue>&& InValue) :
		Type(EValueType::Array)
	{
		Value.Emplace<ArrayType>(MoveTemp(InValue));
	}

	/**
	 * Create an object representing a EValueType::String with the given string value.
	 * 
	 * @param InValue The string value to store.
	 */
	FSignalRValue(const FString& InValue) :
		Type(EValueType::String)
	{
		Value.Set<StringType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::String with the given string value.
	 * 
	 * @param InValue The string value to store (moved).
	 */
	FSignalRValue(FString&& InValue) :
		Type(EValueType::String)
	{
		Value.Emplace<StringType>(MoveTemp(InValue));
	}

	/**
	 * Create an object representing a EValueType::Boolean with the given bool value.
	 * 
	 * @param InValue The boolean value to store.
	 */
	FSignalRValue(bool InValue) :
		Type(EValueType::Boolean)
	{
		Value.Set<BooleanType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Binary with the given array of byte's.
	 * 
	 * @param InValue The binary data to store.
	 */
	FSignalRValue(const TArray<uint8>& InValue) :
		Type(EValueType::Binary)
	{
		Value.Set<BinaryType>(InValue);
	}

	/**
	 * Create an object representing a EValueType::Binary with the given array of byte's.
	 * 
	 * @param InValue The binary data to store (moved).
	 */
	FSignalRValue(TArray<uint8>&& InValue) :
		Type(EValueType::Binary)
	{
		Value.Emplace<BinaryType>(MoveTemp(InValue));
	}

	/**
	 * Copies an existing value.
	 * 
	 * @param OtherValue The value to copy.
	 */
	FSignalRValue(const FSignalRValue& OtherValue)
	{
		Type = OtherValue.Type;
		Value = OtherValue.Value;
	}

	/**
	 * Moves an existing value.
	 * 
	 * @param OtherValue The value to move from.
	 */
	FSignalRValue(FSignalRValue&& OtherValue) noexcept
	{
		Type = MoveTemp(OtherValue.Type);
		Value = MoveTemp(OtherValue.Value);
	}

	/**
	 * Cleans up the resources associated with the value.
	 */
	~FSignalRValue() = default;

	/**
	 * Copies an existing value.
	 * 
	 * @param OtherValue The value to copy.
	 * 
	 * @return Reference to this object.
	 */
	FSignalRValue& operator=(const FSignalRValue& OtherValue)
	{
		Type = OtherValue.Type;
		Value = OtherValue.Value;
		return *this;
	}

	/**
	 * Moves an existing value.
	 * 
	 * @param OtherValue The value to move from.
	 * 
	 * @return Reference to this object.
	 */
	FSignalRValue& operator=(FSignalRValue&& OtherValue) noexcept
	{
		Type = MoveTemp(OtherValue.Type);
		Value = MoveTemp(OtherValue.Value);
		return *this;
	}

	/** @return True if the value is a number, false otherwise. */
	FORCEINLINE bool IsNumber() const
	{
		return Type == EValueType::Number;
	}

	/** @return True if the value is an object, false otherwise. */
	FORCEINLINE bool IsObject() const
	{
		return Type == EValueType::Object;
	}

	/** @return True if the value is a string, false otherwise. */
	FORCEINLINE bool IsString() const
	{
		return Type == EValueType::String;
	}

	/** @return True if the value is null, false otherwise. */
	FORCEINLINE bool IsNull() const
	{
		return Type == EValueType::Null;
	}

	/** @return True if the value is an array, false otherwise. */
	FORCEINLINE bool IsArray() const
	{
		return Type == EValueType::Array;
	}

	/** @return True if the value is a boolean, false otherwise. */
	FORCEINLINE bool IsBoolean() const
	{
		return Type == EValueType::Boolean;
	}

	/** @return True if the value is binary data, false otherwise. */
	FORCEINLINE bool IsBinary() const
	{
		return Type == EValueType::Binary;
	}

	/** @return The EValueType of the stored value. */
	FORCEINLINE EValueType GetType() const
	{
		return Type;
	}

	/** @return The stored value as an int64. */
	FORCEINLINE int64 AsInt() const
	{
		check(Type == EValueType::Number);
		return Value.Get<NumberType>();
	}

	/** @return The stored value as a uint64. */
	FORCEINLINE uint64 AsUInt() const
	{
		check(Type == EValueType::Number);
		return Value.Get<NumberType>();
	}

	/** @return The stored value as a float. */
	FORCEINLINE float AsFloat() const
	{
		check(Type == EValueType::Number);
		return Value.Get<NumberType>();
	}

	/** @return The stored value as a double. */
	FORCEINLINE double AsDouble() const
	{
		check(Type == EValueType::Number);
		return Value.Get<NumberType>();
	}

	/** @return The stored value as a double. */
	FORCEINLINE double AsNumber() const
	{
		check(Type == EValueType::Number);
		return Value.Get<NumberType>();
	}

	/** @return The stored value as a map. */
	FORCEINLINE const TMap<FString, FSignalRValue>& AsObject() const
	{
		check(Type == EValueType::Object);
		return *Value.Get<TSharedPtr<ObjectType>>();
	}

	/** @return The stored value as an array. */
	FORCEINLINE const TArray<FSignalRValue>& AsArray() const
	{
		check(Type == EValueType::Array);
		return Value.Get<ArrayType>();
	}

	/** @return The stored value as a string. */
	FORCEINLINE const FString& AsString() const
	{
		check(Type == EValueType::String);
		return Value.Get<StringType>();
	}

	/** @return The stored value as a boolean. */
	FORCEINLINE bool AsBool() const
	{
		check(Type == EValueType::Boolean);
		return Value.Get<BooleanType>();
	}

	/** @return The stored value as binary data. */
	FORCEINLINE const TArray<uint8>& AsBinary() const
	{
		check(Type == EValueType::Binary);
		return Value.Get<BinaryType>();
	}

private:
	EValueType Type;

	using NumberType = double;
	using ObjectType = TMap<FString, FSignalRValue>;
	using ArrayType = TArray<FSignalRValue>;
	using StringType = FString;
	using BooleanType = bool;
	using BinaryType = TArray<uint8>;
	using FInternalValueVariant = TVariant<NumberType, TSharedPtr<ObjectType>, ArrayType, StringType, BooleanType, BinaryType>;

	FInternalValueVariant Value;
};
