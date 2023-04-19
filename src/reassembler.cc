  std::string adjusted_data = data;
  uint64_t adjusted_first_index = first_index;
  uint64_t first_unass_ind = output.bytes_pushed();

  // completely out
  if (first_index > first_unass_ind + output.available_capacity()) {
    return;
  }
  if (first_index < first_unass_ind) {
    // completely redundant
    if (first_index + data.length() < first_unass_ind) {
      return;
    }
    adjusted_data = adjusted_data.substr(first_unass_ind - first_index, data.length() - (first_unass_ind - first_index));
    adjusted_first_index = first_unass_ind;
  }

  if (adjusted_first_index + adjusted_data.length() > first_unass_ind + output.available_capacity()) {
    adjusted_data = adjusted_data.substr(0, output.available_capacity());
  }

  Element to_insert  = {adjusted_first_index, adjusted_data};
  if (ourSet.empty()){
    ourSet.insert(to_insert);
    return;
  }
  auto lower_bound = ourSet.lower_bound(to_insert);
  auto cur = ourSet.begin();
  auto bound = lower_bound;
  if (adjusted_first_index < lower_bound->index && lower_bound != ourSet.begin()) {
    bound = --lower_bound;
  }
  // completely contined
  if (to_insert.index + to_insert.data.length() < bound->index + bound->data.length()) {
    return;
  }
  if (lower_bound != ourSet.begin()) {
    std::advance(cur, bound);
    Element modif_begin = to_insert;
    uint64_t begin = bound->index + bound->data.length();
    uint64_t segLen = to_insert.index + to_insert.data.length() - begin;
    modif_begin.data = to_insert.data.substr(begin - to_insert.index, segLen);
    modif_begin.index = begin;
    to_insert = modif_begin;
    advance(bound, 1);
  }
    while (bound->index + bound->data.length() < to_insert.index + to_insert.data.length()) {
      
    }

  
  // if (cur->index == to_insert.index ) {
  //   while (cur->index < (to_insert.index + to_insert.data.length())) {
  //     auto prev = cur;
  //     advance(cur, 1);
  //     uint64_t sizeCur = cur->index + cur->data.length();

  //     if (cur->index != prev->index) {
  //       string chunk = to_insert.data.substr(sizeCur, )
  //     }
  //   }
  // }
  
