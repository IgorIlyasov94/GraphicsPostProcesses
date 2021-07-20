#pragma once

#include "stdafx.h"

namespace Graphics
{
	template<typename RingBufferType>
	class RingBufferList
	{
	public:
		RingBufferList() {};
		RingBufferList(size_t bufferSize)
		{
			buffer.resize(bufferSize);
		}

		~RingBufferList() {};

		void PushFront(const RingBufferList& newElement)
		{
			buffer.push_front(newElement);
		}

		void PushFront(RingBufferList&& newElement)
		{
			buffer.push_front(std::forward<RingBufferType>(newElement));
		}

		void PushBack(const RingBufferList& newElement)
		{
			buffer.push_back(newElement);
		}

		void PushBack(RingBufferList&& newElement)
		{
			buffer.push_back(std::forward<RingBufferType>(newElement));
		}

		std::list<RingBufferType>& GetNative() noexcept
		{
			return buffer;
		}

		const size_t& Size() const noexcept
		{
			return buffer.size();
		}

		RingBufferType& operator[](int64_t index)
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			auto bufferIterator = buffer.begin();
			std::advance(bufferIterator, correctIndex);

			return *bufferIterator;
		}

		void RemoveElement(int64_t index) noexcept
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			auto bufferIterator = buffer.begin();
			std::advance(bufferIterator, correctIndex);

			buffer.erase(bufferIterator);
		}

		void Clear() noexcept
		{
			buffer.clear();
		}

	private:
		std::list<RingBufferType> buffer;
	};

	template<typename RingBufferType>
	class RingBufferVector
	{
	public:
		RingBufferVector() {};
		RingBufferVector(size_t bufferSize)
		{
			buffer.resize(bufferSize);
		}

		~RingBufferVector() {};

		void PushBack(const RingBufferVector& newElement)
		{
			buffer.push_back(newElement);
		}

		void PushBack(RingBufferVector&& newElement)
		{
			buffer.push_back(std::forward<RingBufferType>(newElement));
		}

		std::vector<RingBufferType>& GetNative() noexcept
		{
			return buffer;
		}

		size_t Size() const noexcept
		{
			return buffer.size();
		}

		RingBufferType& operator[](int64_t index)
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			return buffer[correctIndex];
		}

		void RemoveElement(int64_t index) noexcept
		{
			size_t correctIndex = (buffer.size() + index) % buffer.size();

			auto bufferIterator = buffer.begin();
			std::advance(bufferIterator, correctIndex);

			buffer.erase(bufferIterator);
		}

		void Clear() noexcept
		{
			buffer.clear();
		}

	private:
		std::vector<RingBufferType> buffer;
	};
}
