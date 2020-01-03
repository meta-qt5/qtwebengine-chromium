// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "cc/base/list_container_helper.h"

#include <stddef.h>

#include <algorithm>
#include <vector>

#include "base/logging.h"
#include "base/memory/aligned_memory.h"

namespace cc {

// PositionInCharAllocator
//////////////////////////////////////////////////////
ListContainerHelper::PositionInCharAllocator::PositionInCharAllocator(
    const ListContainerHelper::PositionInCharAllocator& other) = default;

ListContainerHelper::PositionInCharAllocator::PositionInCharAllocator(
    ListContainerHelper::CharAllocator* container,
    size_t vector_ind,
    char* item_iter)
    : ptr_to_container(container),
      vector_index(vector_ind),
      item_iterator(item_iter) {}

bool ListContainerHelper::PositionInCharAllocator::operator==(
    const ListContainerHelper::PositionInCharAllocator& other) const {
  DCHECK_EQ(ptr_to_container, other.ptr_to_container);
  return vector_index == other.vector_index &&
         item_iterator == other.item_iterator;
}

bool ListContainerHelper::PositionInCharAllocator::operator!=(
    const ListContainerHelper::PositionInCharAllocator& other) const {
  return !(*this == other);
}

ListContainerHelper::PositionInCharAllocator
ListContainerHelper::PositionInCharAllocator::Increment() {
  CharAllocator::InnerList* list =
      ptr_to_container->InnerListById(vector_index);
  if (item_iterator == list->LastElement()) {
    ++vector_index;
    while (vector_index < ptr_to_container->list_count()) {
      if (ptr_to_container->InnerListById(vector_index)->size != 0)
        break;
      ++vector_index;
    }
    if (vector_index < ptr_to_container->list_count())
      item_iterator = ptr_to_container->InnerListById(vector_index)->Begin();
    else
      item_iterator = nullptr;
  } else {
    item_iterator += list->step;
  }
  return *this;
}

ListContainerHelper::PositionInCharAllocator
ListContainerHelper::PositionInCharAllocator::ReverseIncrement() {
  CharAllocator::InnerList* list =
      ptr_to_container->InnerListById(vector_index);
  if (item_iterator == list->Begin()) {
    --vector_index;
    // Since |vector_index| is unsigned, we compare < list_count() instead of
    // comparing >= 0, as the variable will wrap around when it goes out of
    // range (below 0).
    while (vector_index < ptr_to_container->list_count()) {
      if (ptr_to_container->InnerListById(vector_index)->size != 0)
        break;
      --vector_index;
    }
    if (vector_index < ptr_to_container->list_count()) {
      item_iterator =
          ptr_to_container->InnerListById(vector_index)->LastElement();
    } else {
      item_iterator = nullptr;
    }
  } else {
    item_iterator -= list->step;
  }
  return *this;
}

// ListContainerHelper
////////////////////////////////////////////
ListContainerHelper::ListContainerHelper(size_t alignment,
                                         size_t max_size_for_derived_class,
                                         size_t num_of_elements_to_reserve_for)
    : data_(new CharAllocator(alignment,
                              max_size_for_derived_class,
                              num_of_elements_to_reserve_for)) {}

ListContainerHelper::~ListContainerHelper() = default;

void ListContainerHelper::RemoveLast() {
  data_->RemoveLast();
}

void ListContainerHelper::EraseAndInvalidateAllPointers(
    ListContainerHelper::Iterator* position) {
  data_->Erase(position);
}

void ListContainerHelper::InsertBeforeAndInvalidateAllPointers(
    ListContainerHelper::Iterator* position,
    size_t count) {
  data_->InsertBefore(position, count);
}

ListContainerHelper::ConstReverseIterator ListContainerHelper::crbegin() const {
  if (data_->IsEmpty())
    return crend();

  size_t id = data_->LastInnerListId();
  return ConstReverseIterator(data_.get(), id,
                              data_->InnerListById(id)->LastElement(), 0);
}

ListContainerHelper::ConstReverseIterator ListContainerHelper::crend() const {
  return ConstReverseIterator(data_.get(), static_cast<size_t>(-1), nullptr,
                              size());
}

ListContainerHelper::ReverseIterator ListContainerHelper::rbegin() {
  if (data_->IsEmpty())
    return rend();

  size_t id = data_->LastInnerListId();
  return ReverseIterator(data_.get(), id,
                         data_->InnerListById(id)->LastElement(), 0);
}

ListContainerHelper::ReverseIterator ListContainerHelper::rend() {
  return ReverseIterator(data_.get(), static_cast<size_t>(-1), nullptr, size());
}

ListContainerHelper::ConstIterator ListContainerHelper::cbegin() const {
  if (data_->IsEmpty())
    return cend();

  size_t id = data_->FirstInnerListId();
  return ConstIterator(data_.get(), id, data_->InnerListById(id)->Begin(), 0);
}

ListContainerHelper::ConstIterator ListContainerHelper::cend() const {
  if (data_->IsEmpty())
    return ConstIterator(data_.get(), 0, nullptr, size());

  size_t id = data_->list_count();
  return ConstIterator(data_.get(), id, nullptr, size());
}

ListContainerHelper::Iterator ListContainerHelper::begin() {
  if (data_->IsEmpty())
    return end();

  size_t id = data_->FirstInnerListId();
  return Iterator(data_.get(), id, data_->InnerListById(id)->Begin(), 0);
}

ListContainerHelper::Iterator ListContainerHelper::end() {
  if (data_->IsEmpty())
    return Iterator(data_.get(), 0, nullptr, size());

  size_t id = data_->list_count();
  return Iterator(data_.get(), id, nullptr, size());
}

ListContainerHelper::ConstIterator ListContainerHelper::IteratorAt(
    size_t index) const {
  DCHECK_LT(index, size());
  size_t original_index = index;
  size_t list_index;
  for (list_index = 0; list_index < data_->list_count(); ++list_index) {
    size_t current_size = data_->InnerListById(list_index)->size;
    if (index < current_size)
      break;
    index -= current_size;
  }
  return ConstIterator(data_.get(), list_index,
                       data_->InnerListById(list_index)->ElementAt(index),
                       original_index);
}

ListContainerHelper::Iterator ListContainerHelper::IteratorAt(size_t index) {
  DCHECK_LT(index, size());
  size_t original_index = index;
  size_t list_index;
  for (list_index = 0; list_index < data_->list_count(); ++list_index) {
    size_t current_size = data_->InnerListById(list_index)->size;
    if (index < current_size)
      break;
    index -= current_size;
  }
  return Iterator(data_.get(), list_index,
                  data_->InnerListById(list_index)->ElementAt(index),
                  original_index);
}

void* ListContainerHelper::Allocate(size_t alignment,
                                    size_t size_of_actual_element_in_bytes) {
  DCHECK_LE(alignment, data_->alignment());
  DCHECK_LE(size_of_actual_element_in_bytes, data_->element_size());
  return data_->Allocate();
}

size_t ListContainerHelper::size() const {
  return data_->size();
}

bool ListContainerHelper::empty() const {
  return data_->IsEmpty();
}

size_t ListContainerHelper::MaxSizeForDerivedClass() const {
  return data_->element_size();
}

size_t ListContainerHelper::GetCapacityInBytes() const {
  return data_->Capacity() * data_->element_size();
}

void ListContainerHelper::clear() {
  data_->Clear();
}

size_t ListContainerHelper::AvailableSizeWithoutAnotherAllocationForTesting()
    const {
  return data_->NumAvailableElementsInLastList();
}

// ListContainerHelper::Iterator
/////////////////////////////////////////////////
ListContainerHelper::Iterator::Iterator(CharAllocator* container,
                                        size_t vector_ind,
                                        char* item_iter,
                                        size_t index)
    : PositionInCharAllocator(container, vector_ind, item_iter),
      index_(index) {}

ListContainerHelper::Iterator::~Iterator() = default;

size_t ListContainerHelper::Iterator::index() const {
  return index_;
}

// ListContainerHelper::ConstIterator
/////////////////////////////////////////////////
ListContainerHelper::ConstIterator::ConstIterator(
    const ListContainerHelper::Iterator& other)
    : PositionInCharAllocator(other), index_(other.index()) {}

ListContainerHelper::ConstIterator::ConstIterator(CharAllocator* container,
                                                  size_t vector_ind,
                                                  char* item_iter,
                                                  size_t index)
    : PositionInCharAllocator(container, vector_ind, item_iter),
      index_(index) {}

ListContainerHelper::ConstIterator::~ConstIterator() = default;

size_t ListContainerHelper::ConstIterator::index() const {
  return index_;
}

// ListContainerHelper::ReverseIterator
/////////////////////////////////////////////////
ListContainerHelper::ReverseIterator::ReverseIterator(CharAllocator* container,
                                                      size_t vector_ind,
                                                      char* item_iter,
                                                      size_t index)
    : PositionInCharAllocator(container, vector_ind, item_iter),
      index_(index) {}

ListContainerHelper::ReverseIterator::~ReverseIterator() = default;

size_t ListContainerHelper::ReverseIterator::index() const {
  return index_;
}

// ListContainerHelper::ConstReverseIterator
/////////////////////////////////////////////////
ListContainerHelper::ConstReverseIterator::ConstReverseIterator(
    const ListContainerHelper::ReverseIterator& other)
    : PositionInCharAllocator(other), index_(other.index()) {}

ListContainerHelper::ConstReverseIterator::ConstReverseIterator(
    CharAllocator* container,
    size_t vector_ind,
    char* item_iter,
    size_t index)
    : PositionInCharAllocator(container, vector_ind, item_iter),
      index_(index) {}

ListContainerHelper::ConstReverseIterator::~ConstReverseIterator() = default;

size_t ListContainerHelper::ConstReverseIterator::index() const {
  return index_;
}

}  // namespace cc
