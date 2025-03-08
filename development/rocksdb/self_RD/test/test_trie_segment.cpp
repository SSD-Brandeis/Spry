#include<bits/stdc++.h>

using namespace std;

// Use (void) to silence unused warnings.
#define assertm(exp, msg) assert(((void)msg, exp))

struct Node {
    using elementType = unsigned char;
    const static unsigned long SEGMENT_SIZE = 1; //bytes
    const static unsigned long N_BITS_PER_ELEMENT = 8; //bits

    Node** childs;
    int child_count = 0;
    int *fully_covered_counts;
    // string left, right; // 0~(2^bits - 1) (bits = 8*bytes) 
    Node::elementType left_MSB, right_MSB;
    // Node::elementType left_boundary, right_boundary;
    // long long delta;
    int *count_left_subrange, *count_right_subrange, *count_point;

    int *count_bracket_start_left, *count_bracket_start_right, *count_bracket_end_left, *count_bracket_end_right;
    int *count_bracket_start_mid, *count_bracket_end_mid;
    int total_left_bracket_count = 0, total_right_bracket_count = 0;
    // Node(string left, string right){
    Node(){
        // if(left == right){
        //     string info = "Error. left shall < right. " + __FILE__ + ":" + __LINE__ + " " + __FUNCTION__;
        //     assertm(left < right, info);
        //     // childs = NULL;
        //     // counts = NULL;
        // }        
        // else if(stoll(right) - stoll(left) < (1<<8)){
        //     unsigned long long child_num = stoll(right) - stoll(left);
        //     childs = (Node **) malloc(sizeof(Node*)*child_num);
        //     counts = (int*) malloc(sizeof(int) * child_num);
        //     for(int i = 0; i < child_num; i++){counts[i] = 0;}
        // }else{        
        //     int max_child_num = (1 << this->N_SEGMENT);
        //     childs = (Node **) malloc(sizeof(Node*)*max_child_num);
        //     counts = (int*) malloc(sizeof(int) * max_child_num);
        //     for(int i = 0; i < max_child_num; i++){counts[i] = 0;}
        // }

// cout << "A1" << endl;
        // this->left = left;
        // this->right = right;
        // this->left_MSB = stoul(left.substr(0, SEGMENT_SIZE));
        // this->right_MSB = stoul(right.substr(0, SEGMENT_SIZE));
        this->left_MSB = 0;
        // this->right_MSB = 1 << (SEGMENT_SIZE*8);
        this->right_MSB = Node::elementType((1LL << N_BITS_PER_ELEMENT) - 1);
        // this->delta = 
        
        unsigned long long child_num = 1ULL * (this->right_MSB - this->left_MSB);
        unsigned long long max_child_num = (1ULL << (this->SEGMENT_SIZE*8));
        unsigned long long min_num = min(child_num, max_child_num);

        childs = (Node **) malloc(sizeof(Node*)*min_num);
        fully_covered_counts = (int*) calloc(min_num, sizeof(int));

        // this->left_boundary = max_child_num;
        // this->right_boundary = 0;

        count_left_subrange = (int*) calloc(min_num, sizeof(int));
        count_right_subrange = (int*) calloc(min_num, sizeof(int));
        count_point = (int*) calloc(min_num, sizeof(int));
        memset(count_left_subrange, 0, sizeof(int) * min_num);
        memset(count_right_subrange, 0, sizeof(int) * min_num);
        memset(count_point, 0, sizeof(int) * min_num);

        count_bracket_start_left = (int*) calloc(min_num, sizeof(int));
        count_bracket_start_right = (int*) calloc(min_num, sizeof(int));
        count_bracket_end_left = (int*) calloc(min_num, sizeof(int));
        count_bracket_end_right = (int*) calloc(min_num, sizeof(int));
        memset(count_bracket_start_left, 0, sizeof(int) * min_num);
        memset(count_bracket_start_right, 0, sizeof(int) * min_num);
        memset(count_bracket_end_left, 0, sizeof(int) * min_num);
        memset(count_bracket_end_right, 0, sizeof(int) * min_num);

        count_bracket_start_mid = (int*) calloc(min_num, sizeof(int));
        count_bracket_end_mid = (int*) calloc(min_num, sizeof(int));
        memset(count_bracket_start_mid, 0, sizeof(int) * min_num);
        memset(count_bracket_end_mid, 0, sizeof(int) * min_num);

// cout << "A2" << endl;
    }

    static void update_bracket_count(Node* N, unsigned long i_left, unsigned long i_right, unsigned long i_bracket, int start_delta, int end_delta){
        unsigned long i_mid = (i_left + i_right) / 2;
        // N->count_left_bracket[i_mid] += left_delta;
        // N->count_right_bracket[i_mid] += right_delta;
        if(i_mid == i_bracket){
            N->count_bracket_start_mid[i_mid] += start_delta;
            N->count_bracket_end_mid[i_mid] += end_delta;
            return;
        }
        if(i_bracket < i_mid){
            N->count_bracket_start_left[i_mid] += start_delta;
            N->count_bracket_end_left[i_mid] += end_delta;
            update_bracket_count(N, i_left, i_mid-1, i_bracket, start_delta, end_delta);
        }else{
        // if(i_bracket > i_mid){
            N->count_bracket_start_right[i_mid] += start_delta;
            N->count_bracket_end_right[i_mid] += end_delta;
            update_bracket_count(N, i_mid+1, i_right, i_bracket, start_delta, end_delta);
        }
    }

    // static void update_left_bracket_count(Node* N, unsigned long i_left, unsigned long i_right, unsigned long i_bracket){
    //     unsigned long i_mid = (i_left + i_right) / 2;
    //     N->count_left_bracket[i_mid] ++;
    //     if(i_mid == i_bracket){return;}
    //     if(i_bracket < i_mid){
    //         update_left_bracket_count(N, i_left, i_mid-1, i_bracket);
    //     }else{
    //     // if(i_bracket > i_mid){
    //         update_left_bracket_count(N, i_mid+1, i_right, i_bracket);
    //     }
    // }

    // static void update_right_bracket_count(Node* N, unsigned long i_left, unsigned long i_right, unsigned long i_bracket){
    //     unsigned long i_mid = (i_left + i_right) / 2;
    //     N->count_right_bracket[i_mid] ++;
    //     if(i_mid == i_bracket){return;}
    //     if(i_bracket < i_mid){
    //         update_right_bracket_count(N, i_left, i_mid-1, i_bracket);
    //     }else{
    //     // if(i_bracket > i_mid){
    //         update_right_bracket_count(N, i_mid+1, i_right, i_bracket);
    //     }
    // }
    static void increase_left_bracket_count(Node *N, unsigned long i_bracket){
        unsigned long i_left = N->left_MSB;
        unsigned long i_right = N->right_MSB;
        N->total_left_bracket_count ++;

        assertm(i_left <= i_bracket && i_bracket <= i_right, ("i_bracket (" + to_string(i_bracket) + ") shall be in [" + to_string(i_left) + ", " + to_string(i_right) + "]"));
        update_bracket_count(N, i_left, i_right, i_bracket, 1, 0);
    }

    static void increase_right_bracket_count(Node *N, unsigned long i_bracket){
        unsigned long i_left = N->left_MSB;
        unsigned long i_right = N->right_MSB;
        N->total_right_bracket_count ++;

        assertm(i_left <= i_bracket && i_bracket <= i_right, ("i_bracket (" + to_string(i_bracket) + ") shall be in [" + to_string(i_left) + ", " + to_string(i_right) + "]"));
        update_bracket_count(N, i_left, i_right, i_bracket, 0, 1);
    }

    static void decrease_left_bracket_count(Node *N, unsigned long i_bracket){
        unsigned long i_left = N->left_MSB;
        unsigned long i_right = N->right_MSB;
        N->total_left_bracket_count --;

        assertm(i_left <= i_bracket && i_bracket <= i_right, ("i_bracket (" + to_string(i_bracket) + ") shall be in [" + to_string(i_left) + ", " + to_string(i_right) + "]"));
        update_bracket_count(N, i_left, i_right, i_bracket, -1, 0);
    }

    static void decrease_right_bracket_count(Node *N, unsigned long i_bracket){
        unsigned long i_left = N->left_MSB;
        unsigned long i_right = N->right_MSB;
        N->total_right_bracket_count --;

        assertm(i_left <= i_bracket && i_bracket <= i_right, ("i_bracket (" + to_string(i_bracket) + ") shall be in [" + to_string(i_left) + ", " + to_string(i_right) + "]"));
        update_bracket_count(N, i_left, i_right, i_bracket, 0, -1);
    }

    static void walk_through_bracket_count(Node *N){
        print_bracket_count(N, N->left_MSB, N->right_MSB);
    }
    static void print_bracket_count(Node *N, unsigned long i_left, unsigned long i_right){
        if(i_left >= i_right){
            return;
        }
        unsigned long i_mid = (i_left + i_right) / 2;
        cout << "[" << i_mid << "]" << ", start_bracket_left_count = " << N->count_bracket_start_left[i_mid] << ", start_bracket_right_count = " << N->count_bracket_start_right[i_mid] << \
                ", end_bracket_left_count = " << N->count_bracket_end_left[i_mid] << ", end_bracket_right_count = " << N->count_bracket_end_right[i_mid] << \
                ", start_bracket_mid_count = " << N->count_bracket_start_mid[i_mid] << ", end_bracket_mid_count = " << N->count_bracket_end_mid[i_mid] << endl;
        print_bracket_count(N, i_left, i_mid-1);
        print_bracket_count(N, i_mid+1, i_right);
    }

    //[0, i_bracket]
    static unsigned long accumulate_left_bracket_count(Node* N, unsigned long i_left, unsigned long i_right, unsigned long i_bracket){
        unsigned long i_mid = (i_left + i_right) / 2;
        unsigned long count = N->count_bracket_start_left[i_mid] + N->count_bracket_start_mid[i_mid];
        if(i_mid == i_bracket){return count;}
        if(i_bracket < i_mid){
            return accumulate_left_bracket_count(N, i_left, i_mid-1, i_bracket);
        }else{
        // if(i_bracket > i_mid){
            return count + accumulate_left_bracket_count(N, i_mid+1, i_right, i_bracket);
        }
    }

    //[0, i_bracket]
    static unsigned long accumulate_right_bracket_count(Node* N, unsigned long i_left, unsigned long i_right, unsigned long i_bracket){
        unsigned long i_mid = (i_left + i_right) / 2;
        unsigned long count = N->count_bracket_end_left[i_mid] + N->count_bracket_end_mid[i_mid];
        if(i_mid == i_bracket){return count;}
        if(i_bracket < i_mid){
            return accumulate_right_bracket_count(N, i_left, i_mid-1, i_bracket);
        }else{
        // if(i_bracket > i_mid){
            return count + accumulate_right_bracket_count(N, i_mid+1, i_right, i_bracket);
        }
    }

    static bool is_point_covered_by_brackets(Node* N, unsigned long i_input){
        long accumulated_left_bracket_count = accumulate_left_bracket_count(N, N->left_MSB, N->right_MSB, i_input);
        long accumulated_right_bracket_count = accumulate_right_bracket_count(N, N->left_MSB, N->right_MSB, i_input);
        long left_bracket_count = N->count_bracket_start_mid[i_input];
        long right_bracket_count = N->count_bracket_end_mid[i_input];
        // if(accumulated_left_bracket_count - left_bracket_count > accumulated_right_bracket_count){
        if(accumulated_left_bracket_count > accumulated_right_bracket_count - right_bracket_count){
// cout << "accumulated_left_bracket_count: " << accumulated_left_bracket_count << endl;
// cout << "accumulated_right_bracket_count: " << accumulated_right_bracket_count << endl;
// cout << "left_bracket_count: " << left_bracket_count << endl;
// cout << "right_bracket_count: " << right_bracket_count << endl;
            return true;
        }
        return false;
    }



    static void update_subrange_count(Node *N, unsigned long i_left, unsigned long i_right, unsigned long input_left_MSB, unsigned long input_right_MSB, int delta){
        if(input_left_MSB == input_right_MSB){
            N->count_point[input_left_MSB] += delta;
            return;
        }

        if(i_left >= i_right){
            return;
        }

        // if(i_right <= input_left_MSB || i_left >= input_right_MSB){
        //     return;
        // }

        // if(i_left == i_right){
        //     unsigned long i_mid = i_left;
        //     // if(i_mid != 0 && i_mid-1 <= input_left_MSB && input_right_MSB <= i_mid){
        //     if(i_mid > 1UL*N->left_MSB && i_mid-1 <= input_left_MSB && input_right_MSB <= i_mid){
        //         N->count_left_subrange[i_mid] += delta;
        //     }
        //     if(i_mid < 1UL*N->right_MSB && i_mid <= input_left_MSB && input_right_MSB <= i_mid+1){
        //         N->count_right_subrange[i_mid] += delta;
        //     }
        //     return;
        // }

        // long long i_left = N->left_MSB;
        // long long i_right = N->right_MSB;

        unsigned long i_mid = (i_left + i_right) / 2;
        unsigned long i_left_mid = (i_left + i_mid) / 2;
        unsigned long i_right_mid = (i_mid + i_right) / 2;
        unsigned long a = input_left_MSB;
        unsigned long b = min(input_right_MSB, i_mid);

        if(i_right_mid == i_mid && i_right_mid < i_right){i_right_mid ++;}

// cout << i_left << " " << i_mid << " " << i_right << " " << input_left_MSB << " " << input_right_MSB << " " << "pp" << endl;   
// cout << i_left_mid << " " << i_mid << " " << i_right_mid << " " << input_left_MSB << " " << input_right_MSB << " " << "p" << endl;   
        if(a < b && i_left_mid < i_mid){
            if(a <= i_left_mid && i_mid == b){
                N->count_left_subrange[i_mid] += delta;
// cout << "update" << "[" << i_left_mid << ", " << i_mid << "] = " << N->count_left_subrange[i_mid] << endl;
                if(a < i_left_mid){
                    update_subrange_count(N, i_left, i_mid, a, i_left_mid, delta);
                }
            }else{
                update_subrange_count(N, i_left, i_mid, a, b, delta);
            }
        }

        a = max(input_left_MSB, i_mid);
        b = input_right_MSB;
        if(a < b && i_mid < i_right_mid){
            if(i_mid == a && b >= i_right_mid){
                N->count_right_subrange[i_mid] += delta;
// cout << "update" << "[" << i_mid << ", " << i_right_mid << "] = " << N->count_right_subrange[i_mid] << endl;

                if(b > i_right_mid){
                    update_subrange_count(N, i_mid, i_right, i_right_mid, b, delta);
                }
            }else{
                update_subrange_count(N, i_mid, i_right, a, b, delta);
            }
        }
        
    }

    bool is_covered_by_subrange(Node* N, unsigned long i_left, unsigned long i_right, unsigned long input_MSB){
        if(N->count_point[input_MSB] > 0){
// cout << "B1" << endl;
            return true;
        }

        if(i_left >= i_right){
// cout << "B2" << endl;
            return false;
        }
        if(i_left + 1 == i_right){
            unsigned long i_mid = i_left;
            return N->count_right_subrange[i_mid] > 0;
        }

        // if(i_left == i_right){
        //     unsigned long i_mid = i_left;
        //     // if(i_mid != 0 && i_mid-1 <= input_left_MSB && input_right_MSB <= i_mid){
        //     if(i_mid > 1UL*N->left_MSB && i_mid-1 <= input_MSB && input_MSB <= i_mid){
        //         return N->count_left_subrange[i_mid] > 0;
        //     }
        //     if(i_mid < 1UL*N->right_MSB && i_mid <= input_MSB && input_MSB <= i_mid+1){
        //         return N->count_right_subrange[i_mid] > 0;
        //     }
        //     return false;
        // }

        // long long i_left = N->left_MSB;
        // long long i_right = N->right_MSB;

        unsigned long i_mid = (i_left + i_right) / 2;
        unsigned long i_left_mid = (i_left + i_mid) / 2;
        unsigned long i_right_mid = (i_mid + i_right) / 2;
        // unsigned long a = input_MSB;

        if(i_right_mid == i_mid && i_right_mid < i_right){i_right_mid ++;}

// cout << "i_left: " << i_left <<  " i_mid = " << i_mid << " i_right = " << i_right << " input_MSB = " << input_MSB << endl;
// cout << "i_left_mid: " << i_left_mid <<  " i_mid = " << i_mid << " i_right_mid = " << i_right_mid << " input_MSB = " << input_MSB << endl;

        if(input_MSB == i_mid){
// cout << "B3" << endl;
            if(N->count_left_subrange[i_mid] > 0 || N->count_right_subrange[i_mid] > 0){
// cout << "B30" << endl;
                return true;
            }

            if(i_left < i_mid && i_left+1 < i_mid){
                if(is_covered_by_subrange(N, i_left, i_mid, input_MSB)){
// cout << "B31" << endl;
                    return true;
                }
            }
            
            if(i_mid < i_right && i_mid+1 < i_right){
                if(is_covered_by_subrange(N, i_mid, i_right, input_MSB)){
// cout << "B32" << endl;
                    return true;
                }
            }            
        }

        if(input_MSB < i_mid){
            if(i_left_mid < i_mid && i_left_mid <= input_MSB && N->count_left_subrange[i_mid] > 0){
// cout << "B4" << endl;
                return true;
            }
            
            if(i_left < i_mid && i_left+1 < i_mid){
                if(is_covered_by_subrange(N, i_left, i_mid, input_MSB)){
    // cout << "B5" << endl;
                    return true;
                }   
            }    
        }

        if(input_MSB > i_mid){
            if(i_right_mid > i_mid && i_right_mid >= input_MSB && N->count_right_subrange[i_mid] > 0){
// cout << "B6" << endl;
                return true;
            }
            if(i_mid < i_right && i_mid+1 < i_right){
                if(is_covered_by_subrange(N, i_mid, i_right, input_MSB)){
    // cout << "B7" << endl;
                    return true;
                }       
            }
        }

        return false;
    }

    static void increase_subrange_count(Node* N, unsigned long i_left, unsigned long i_right, unsigned long input_left_MSB, unsigned long input_right_MSB){
        update_subrange_count(N, i_left, i_right, input_left_MSB, input_right_MSB, 1);
    }

    static void decrease_subrange_count(Node* N, unsigned long i_left, unsigned long i_right, unsigned long input_left_MSB, unsigned long input_right_MSB){
        update_subrange_count(N, i_left, i_right, input_left_MSB, input_right_MSB, -1);
    }

    static void walk_through_subrange_count(Node *N){
        print_subrange_count(N, N->left_MSB, N->right_MSB, N->left_MSB, N->right_MSB);
    }

    static void print_subrange_count(Node *N, unsigned long i_left, unsigned long i_right, unsigned long input_left_MSB, unsigned long input_right_MSB){
// cout << i_left << " " << i_right << " " << input_left_MSB << " " << input_right_MSB << " " << "p" << endl;

        if(i_left >= i_right){
// cout << i_left << " " << i_right << " " << input_left_MSB << " " << input_right_MSB << " " << "i_left >= i_right" << endl;
            return;
        }
//         // if(i_left < i_right && (i_right <= input_left_MSB || i_left >= input_right_MSB)){
//         if(i_right <= input_left_MSB || i_left >= input_right_MSB){
// // cout << i_left << " " << i_right << " " << input_left_MSB << " " << input_right_MSB << " " << "afde" << endl;
//             return;
//         }

        unsigned long i_mid = (i_left + i_right) / 2;
        unsigned long i_left_mid = (i_left + i_mid) / 2;
        unsigned long i_right_mid = (i_mid + i_right) / 2;

        if(i_left < i_mid && i_left+1 < i_mid && i_left+2 < i_mid && i_left+3 < i_mid){
            cout << "[" << i_left_mid << ", " << i_mid << "] = " << N->count_left_subrange[i_mid] << endl;
            print_subrange_count(N, i_left, i_mid, input_left_MSB, i_mid);
        }else if(i_left < i_mid && i_left+1 < i_mid && i_left+2 < i_mid && i_left+3 == i_mid){
            cout << "[" << i_left_mid << ", " << i_mid << "] = " << N->count_left_subrange[i_mid] << endl;
            print_subrange_count(N, i_left, i_mid, input_left_MSB, i_left_mid); // i_left_mid == i_mid - 1
        }else if(i_left < i_mid && i_left+1 < i_mid && i_left+2 == i_mid){
            cout << "[" << i_left << ", " << i_left_mid << "] = " << N->count_left_subrange[i_left_mid] << endl;
            cout << "[" << i_left_mid << ", " << i_mid << "] = " << N->count_left_subrange[i_mid] << endl;
        }else if(i_left < i_mid && i_left+1 == i_mid){
            cout << "[" << i_left_mid << ", " << i_mid << "] = " << N->count_left_subrange[i_mid] << endl;
        }

        if(i_mid < i_right && i_mid+1 < i_right && i_mid+2 < i_right && i_mid+3 < i_right){
            cout << "[" << i_mid << ", " << i_right_mid << "] = " << N->count_right_subrange[i_mid] << endl;
            print_subrange_count(N, i_mid, i_right, i_mid, input_right_MSB);
        }else if(i_mid < i_right && i_mid+1 < i_right && i_mid+2 < i_right && i_mid+3 == i_right){
            print_subrange_count(N, i_mid, i_right, i_right_mid, input_right_MSB); //i_right_mid = i_mid + 1
            cout << "[" << i_mid << ", " << i_right_mid << "] = " << N->count_right_subrange[i_mid] << endl;
        }else if(i_mid < i_right && i_mid+1 < i_right && i_mid+2 == i_right){
            cout << "[" << i_mid << ", " << i_right_mid << "] = " << N->count_right_subrange[i_mid] << endl;
            cout << "[" << i_right_mid << ", " << i_right << "] = " << N->count_right_subrange[i_right_mid] << endl;
        }else if(i_mid < i_right && i_mid + 1 == i_right){
            cout << "[" << i_mid << ", " << i_right_mid << "] = " << N->count_right_subrange[i_mid] << endl;
        }

        // if(i_left < i_mid && i_left+1 < i_mid){
        //     cout << "[" << i_left << ", " << i_mid << "] = " << N->count_left_subrange[i_mid] << endl;
        //     print_subrange_count(N, i_left, i_mid, input_left_MSB, i_mid);
        // }else if(i_left == i_mid-1){
        //     cout << "[" << i_left << ", " << i_mid << "] = " << N->count_left_subrange[i_mid] << endl;
        // }

        // if(i_mid < i_right && i_mid+1 < i_right){
        //     cout << "[" << i_mid << ", " << i_right << "] = " << N->count_right_subrange[i_mid] << endl;
        //     print_subrange_count(N, i_mid, i_right, i_mid, input_right_MSB);
        // }else if(i_mid + 1 == i_right){
        //     cout << "[" << i_mid << ", " << i_right << "] = " << N->count_right_subrange[i_mid] << endl;
        // }
    }
    
    // static unsigned long long string2num(string str){
    //     unsigned long long num = 0;
    //     for(int i = 0; i < str.size(); i++){
    //         // num *= (1 << (SEGMENT_SIZE*8)); 
    //         num <<= (SEGMENT_SIZE*8); 
    //         num += str[i];
    //     }
    //     return num;
    // }

    static vector<Node::elementType> string2BitArray(string str, int N_bits_per_element){
        assert((void((string("N_bits_per_element ") + to_string(N_bits_per_element) + " shall be >= 1 and <= " + to_string(sizeof(Node::elementType))).c_str()),
                N_bits_per_element >= 1 && N_bits_per_element <= sizeof(Node::elementType)*8));
        int N_bits = (str.size() * 8) / N_bits_per_element + ((str.size() * 8) % N_bits_per_element > 0);
    // cout << "N_bits = " << N_bits << endl;  
        // char *bit_array = (char*) malloc(sizeof(char) * N_bits);
        // memset(bit_array, 0, sizeof(char) * N_bits);
        vector<Node::elementType> bit_array(N_bits, 0);

        int accumulated_bits = 0;
        int N_accumulated_bits = 0;
        int j = 0;
        for(int i = 0; i < str.size(); i++){
            accumulated_bits <<= sizeof(str[i]) * 8;
            accumulated_bits += str[i];
            N_accumulated_bits += sizeof(str[i]) * 8;
            while(N_accumulated_bits >= N_bits_per_element){
                bit_array[j] = (accumulated_bits >> (N_accumulated_bits - N_bits_per_element)) % (1 << N_bits_per_element);
                // accumulated_bits >>= N_bits_per_element;  
                N_accumulated_bits -= N_bits_per_element;
                j++;
            }
            accumulated_bits %= (1 << N_accumulated_bits);
        }
        if(N_accumulated_bits > 0){
            bit_array[j] = accumulated_bits << (N_bits_per_element - N_accumulated_bits);
        }

        for(int i = bit_array.size()-1; i >= 0; i--){
            if(bit_array[i] != 0){
                break;
            }
            bit_array.pop_back();
        }

        return bit_array;
    }



    // static pair<unsigned long, string> get_MSB(string str){
    //     string LSB_str;
    //     int N_bits = SEGMENT_SIZE*8;
    //     unsigned long long num = 0;
    //     int bit_count = 0;
    //     int i = 0;
    //     for(; i < str.size(); i++){
    //         num *= (1 << 8); 
    //         num += str[i];
    //         bit_count += 8;
    //         if(bit_count >= N_bits){
    //             break;
    //         }
    //     }
    //     if(N_bits > bit_count){
    //         num *= (1 << (N_bits - bit_count));
    //         bit_count += (N_bits - bit_count);
    //     }
    //     unsigned long long MSB = num / (1 << (bit_count - N_bits));
    //     num %= (1 << (bit_count - N_bits));
    //     bit_count -= N_bits;

    //     for(;i < str.size(); i++){
    //         num *= (1 << 8); 
    //         num += str[i];
    //         bit_count += 8;

    //         LSB_str += (char) (num / (1 << (bit_count - 8)));
    //         num %= (1 << (bit_count - 8));
    //         bit_count -= 8;
    //     }
    //     LSB_str += (char) (num);

    //     return make_pair(MSB, LSB_str);
    // }
    
    // static long long get_delta(Node* N){
    //     unsigned long left_MSB = N->left_MSB;
    //     unsigned long right_MSB = N->right_MSB;
    //     unsigned long num = N->SEGMENT_SIZE;
    //     // return  (right_MSB - left_MSB +1)/num - 1;
    //     return  (right_MSB - left_MSB +1)/num;
    // }

    static void insert_left_bracket(Node *N, vector<Node::elementType> &input, unsigned int depth){
        if(N == NULL){assert(false);}
        if(depth == input.size()){assert(false);}

        unsigned long idx = input[depth];
        N->increase_left_bracket_count(N, idx);
        N->child_count++;

        if(depth == input.size() - 1){
            return;
        }

        if(N->childs[idx] == NULL){
            N->childs[idx] = new Node();
        }
        insert_left_bracket(N->childs[idx], input, depth+1);
    }

    static void insert_right_bracket(Node *N, vector<Node::elementType> &input, unsigned int depth){
        if(N == NULL){assert(false);}
        if(depth == input.size()){assert(false);}

        unsigned long idx = input[depth];
        N->increase_right_bracket_count(N, idx);
        N->child_count++;

        if(depth == input.size() - 1){
            return;
        }

        if(N->childs[idx] == NULL){
            N->childs[idx] = new Node();
        }
        insert_right_bracket(N->childs[idx], input, depth+1);
    }

    static void insert_brackets(Node* N, vector<Node::elementType> &input_left, vector<Node::elementType> &input_right){
        unsigned int depth = 0;
        insert_left_bracket(N, input_left, depth);
        insert_right_bracket(N, input_right, depth);
    }

    static void delete_left_bracket(Node *N, vector<Node::elementType> &input, unsigned int depth){
        if(N == NULL){assert(false);}
        if(depth == input.size()){assert(false);}

        unsigned long idx = input[depth];
        N->decrease_left_bracket_count(N, idx);
        N->child_count--;

        if(depth == input.size() - 1){
            return;
        }

        if(N->childs[idx] == NULL){
            assertm(false, "Error: can only deleted already inserted ones. This bracket should exist.");
        }
        delete_left_bracket(N->childs[idx], input, depth+1);
    
        if(N->childs[idx]->child_count == 0){
            delete N->childs[idx];
            N->childs[idx] = NULL;
        }
    }

    static void delete_right_bracket(Node *N, vector<Node::elementType> &input, unsigned int depth){
        if(N == NULL){assert(false);}
        if(depth == input.size()){assert(false);}

        unsigned long idx = input[depth];
        N->decrease_right_bracket_count(N, idx);
        N->child_count--;

        if(depth == input.size() - 1){
            return;
        }

        if(N->childs[idx] == NULL){
            assertm(false, "Error: can only deleted already inserted ones. This bracket should exist.");
        }
        delete_right_bracket(N->childs[idx], input, depth+1);
    
        if(N->childs[idx]->child_count == 0){
            delete N->childs[idx];
            N->childs[idx] = NULL;
        }
    }

    static void delete_brackets(Node* N, vector<Node::elementType> &input_left, vector<Node::elementType> &input_right){
        unsigned int depth = 0;
        delete_left_bracket(N, input_left, depth);
        delete_right_bracket(N, input_right, depth);
    }

    static bool is_covered_by_brackets(Node *N, vector<Node::elementType> &input){
        return is_covered_by_brackets(N, input, 0, 0, 0);
    }

    static bool is_covered_by_brackets(Node *N, vector<Node::elementType> &input, unsigned int depth, unsigned long prev_left_bracket_count, unsigned long prev_right_bracket_count){
        if(N == NULL){assert(false);}
        if(depth == input.size()){assert(false);}

        unsigned long idx = input[depth];
        long accumulated_left_bracket_count = accumulate_left_bracket_count(N, N->left_MSB, N->right_MSB, idx);
        long accumulated_right_bracket_count = accumulate_right_bracket_count(N, N->left_MSB, N->right_MSB, idx);
        long left_bracket_count = N->count_bracket_start_mid[idx];
        long right_bracket_count = N->count_bracket_end_mid[idx];

// cout << "idx = " << idx << " depth = " << depth << endl;
// cout << "accumulated_left_bracket_count = " << accumulated_left_bracket_count << endl;
// cout << "accumulated_right_bracket_count = " << accumulated_right_bracket_count << endl;
// cout << "left_bracket_count = " << left_bracket_count << endl;
// cout << "right_bracket_count = " << right_bracket_count << endl;
// cout << "prev_left_bracket_count = " << prev_left_bracket_count << endl;
// cout << "prev_right_bracket_count = " << prev_right_bracket_count << endl;   

        //MSB is certain to be covered by some brackets
        if(prev_left_bracket_count + accumulated_left_bracket_count - left_bracket_count > prev_right_bracket_count + accumulated_right_bracket_count){
// cout << "xa3" << endl;
            return true;
        }

        if(depth == input.size() - 1 && N->childs[idx] == NULL){
// cout << "xa4" << endl;
            if(prev_left_bracket_count + accumulated_left_bracket_count - left_bracket_count > prev_right_bracket_count + accumulated_right_bracket_count - right_bracket_count){
                return true;
            }
            if(left_bracket_count > 0 || right_bracket_count > 0){
                return true;
            }
            return false;
        }

        if(N->childs[idx] == NULL){ 
            //depth < input.size() - 1
// cout << "xa1" << endl;
            if(prev_left_bracket_count + accumulated_left_bracket_count > prev_right_bracket_count + accumulated_right_bracket_count){
                return true;
            }
            // // sum left_bracket == sum right_bracket
            // // xxx 000 000 000 000 ...
            // if(left_bracket_count > 0){
            //     return true;
            // }

            return false;
        }

        if(depth == input.size() - 1){
            //N->childs[idx] != NULL
// cout << "xa2" << endl;
            if(prev_left_bracket_count + accumulated_left_bracket_count - left_bracket_count > prev_right_bracket_count + accumulated_right_bracket_count - right_bracket_count){
                return true;
            }
            // sum left_bracket == sum right_bracket
            // xxx 000 000 000 000 ...
            if(left_bracket_count > N->childs[idx]->total_left_bracket_count){
                return true;
            }
            return false;
        }

        
        unsigned long next_prev_left_bracket_count = prev_left_bracket_count + accumulated_left_bracket_count - N->childs[idx]->total_left_bracket_count;
        unsigned long next_prev_right_bracket_count = prev_right_bracket_count + accumulated_right_bracket_count - N->childs[idx]->total_right_bracket_count;
        return is_covered_by_brackets(N->childs[idx], input, depth+1, next_prev_left_bracket_count, next_prev_right_bracket_count);
    }
};

// // void insert(Node* N, string start, string end, int offset){
// void insert_range(Node* N, vector<Node::elementType> input_left, Node::elementType input_left_idx_0, vector<Node::elementType> input_right, Node::elementType input_right_idx_max, int depth){
// cout << "B1" << endl;
//     if(N == NULL){
//         string info1 = "Error. Shall not be NULL. " + string(__FILE__) + ":" + to_string(__LINE__) + " " + __FUNCTION__;
//         assertm(N != NULL, info1); 
//     }
//     if(N->left_MSB > N->right_MSB){
//         // string info2 = "Error. left_8 shall <= right_8. " + __FILE__ + ":" + __LINE__ + " " + __FUNCTION__;
        
//         std::ostringstream oss;
//         oss << "Error. left_8 shall <= right_8. " << string(__FILE__) << ":" << to_string(__LINE__) << " " << __FUNCTION__;
//         std::string info2 = oss.str();

//         assertm(N->left_MSB <= N->right_MSB, info2); 
//     }
//     // if(N->left >= N->right){
//     //     string info3 = "Error. left shall < right. " + __FILE__ + ":" + __LINE__ + " " + __FUNCTION__;
//     //     assertm(N->left < N->right, info3); 
//     // }
// // cout << "input_left: " << input_left << " " << input_left.size() << endl;
// // cout << "input_right: " << input_right << " " << input_right.size() << endl;
//     // string input_left_MSB_str = input_left.substr(0, N->SEGMENT_SIZE); 
//     // string input_right_MSB_str = input_right.substr(0, N->SEGMENT_SIZE); 
//     // cout << "input_left_MSB_str: " << input_left_MSB_str << endl;
//     // cout << "input_right_MSB_str: " << input_right_MSB_str << endl;
//     // unsigned long input_left_MSB = stoul(input_left_MSB_str);
//     // unsigned long input_right_MSB = stoul(input_right_MSB_str);
//     Node::elementType input_left_MSB = input_left[depth];
//     Node::elementType input_right_MSB = input_right[depth];
//     cout << "input_left_MSB: " << input_left_MSB << endl;
//     cout << "input_right_MSB: " << input_right_MSB << endl;

//     unsigned long left_MSB = N->left_MSB;
//     unsigned long right_MSB = N->right_MSB;
//     // unsigned long delta = N->get_delta(N);


//     // update left_boundary and right_boundary
//     N->left_boundary = min(N->left_boundary, input_left_MSB);
//     N->right_boundary = max(N->right_boundary, input_right_MSB);

//     // mark fully covered regions
//     // unsigned long long start = left_MSB + delta * ((input_left_MSB - left_MSB)/delta);
//     // unsigned long long end = left_MSB + delta -1;

//     // unsigned long idx_left = ((input_left_MSB - left_MSB)/delta);
//     // unsigned long idx = idx_left-1;
//     // for(; start < input_right_MSB; start += delta){ // startt <= 4 bytes
//     //     idx++;
//     //     end = start + delta   - 1;
//     //     if(start >= input_left_MSB && end <= input_right_MSB){
//     //         N->fully_covered_counts[idx] ++;
//     //     }
//     // }
//     // // end = start + delta - 1;
//     // unsigned long idx_right = idx;
//     unsigned long long start = input_left_MSB;
//     unsigned long long end = input_right_MSB;

//     for(int i = start+1; i < end; i++){
//         N->fully_covered_counts[i] ++;
//     }
//     if(input_left_MSB < input_right_MSB){
//         // if(input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE) == string(N->SEGMENT_SIZE, 0)){
//         if(depth >= input_left_idx_0){
//             N->fully_covered_counts[input_left_MSB] ++;
//         }
//         // if(input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE) == string(N->SEGMENT_SIZE, 255)){
//         if(depth <= input_right_idx_max){
//             N->fully_covered_counts[input_right_MSB] ++;
//         }
//     }


//     //recursively insert
//     unsigned long idx_left = left_MSB;
//     unsigned long idx_right = right_MSB;

//     if(input_left_MSB_str.size() >= N->SEGMENT_SIZE){
//         // input_left and input_right falls in the same sub-region
//         if(idx_left == idx_right){
//             unsigned long idx = idx_left;
             
//             if(N->childs[idx] == NULL){
//                 // N->childs[idx] = (Node*) new Node(string(N->SEGMENT_SIZE, 0), string(N->SEGMENT_SIZE, 255));
//                 N->childs[idx] = (Node*) new Node();
//             }
// cout << "B1.0" << endl;
// cout << "input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE): " << input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE) << endl;
// cout << "input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE): " << input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE) << endl;
//             insert_range(N->childs[idx], input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE), 
//                 input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE));
        
//             return;
//         }

//         //insert input_left
//         if(N->childs[idx_left] == NULL){
//             // N->childs[idx_left] = (Node*) new Node(string(N->SEGMENT_SIZE, 0), string(N->SEGMENT_SIZE, 255));
//             N->childs[idx_left] = (Node*) new Node();
//         }
// cout << "B1.1" << endl;
// cout << "input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE): " << input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE) << endl;
// cout << "input_right " <<  string(N->SEGMENT_SIZE, 255) << endl;
//         insert_range(N->childs[idx_left], input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE), 
//             string(N->SEGMENT_SIZE, 255));

//         //insert input_right
//         if(N->childs[idx_right] == NULL){
//             // N->childs[idx_left] = (Node*) new Node(string(N->SEGMENT_SIZE, 0), string(N->SEGMENT_SIZE, 255));
//             N->childs[idx_right] = (Node*) new Node();
//         }
// cout << "B1.2" << endl;
// cout << "input_left " << string(N->SEGMENT_SIZE, 0) << endl;
// cout << "input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE): " << input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE) << endl;
//         insert_range(N->childs[idx_right], string(N->SEGMENT_SIZE, 0),
//             input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE));
        
//         return;
//     }

//     N->fully_covered_counts[idx_left] ++;
//     N->fully_covered_counts[idx_right] ++;
// cout << "B2" << endl;
// }

// bool search_pq(Node* N, string input){
//     if(N == NULL){
//         return true;
//     }

//     string input_MSB_str = input.substr(0, N->SEGMENT_SIZE);
//     unsigned long input_MSB = stoul(input_MSB_str);

//     if(input_MSB < N->left_boundary || input_MSB > N->right_boundary){
//         return true;
//     }
    
//     // unsigned long left_MSB = N->left_MSB;
//     // unsigned long right_MSB = N->right_MSB;
//     // unsigned long delta = N->get_delta(N);

//     // int idx = (input_MSB - N->left_MSB)/delta;
//     int idx = input_MSB;
//     if(N->fully_covered_counts[idx] > 0){
//         return false;
//     }

//     if(input_MSB_str.size() >= N->SEGMENT_SIZE){
//         return search_pq(N->childs[idx], input.substr(N->SEGMENT_SIZE, input.size() - N->SEGMENT_SIZE));
//     }

//     return true;
// }


// void remove_range(Node* N, string input_left, string input_right){
//     if(N == NULL){
//         string info1 = "Error. Shall not be NULL. " + string(__FILE__) + ":" + to_string(__LINE__) + " " + __FUNCTION__;
//         assertm(N != NULL, info1); 
//     }

//     string input_left_MSB_str = input_left.substr(0, N->SEGMENT_SIZE);
//     string input_right_MSB_str = input_right.substr(0, N->SEGMENT_SIZE);
//     unsigned long input_left_MSB = stoul(input_left_MSB_str);
//     unsigned long input_right_MSB = stoul(input_right_MSB_str);

//     unsigned long left_MSB = N->left_MSB;
//     unsigned long right_MSB = N->right_MSB;
//     // unsigned long delta = N->get_delta(N);


//     // decrease fully covered regions
//     // unsigned long long start = left_MSB + delta * ((input_left_MSB - left_MSB)/delta);
//     // unsigned long long end = left_MSB + delta -1;
    
//     // unsigned long idx_left = ((input_left_MSB - left_MSB)/delta);
//     // unsigned long idx = idx_left-1;
//     // for(; start < input_right_MSB; start += delta){ // startt <= 4 bytes
//     // {
//     //     idx++;
//     //     end = start + delta - 1;
//     //     if(start >= input_left_MSB && end <= input_right_MSB){
//     //         N->fully_covered_counts[(start - left_MSB)/delta] --;

//     //         string flag = "Error. Shall >= 0. " + __FILE__ + ":" + __LINE__ + " " + __FUNCTION__;
//     //         assertm(N->fully_covered_counts[(start - left_MSB)/delta] >= 0, flag);
//     //     }
//     // }
//     // unsigned long idx_right = idx;
//     unsigned long long start = input_left_MSB;
//     unsigned long long end = input_right_MSB;

//     for(int i = start+1; i < end; i++){
//         N->fully_covered_counts[i] --;
//     }
//     if(input_left_MSB < input_right_MSB){
//         if(input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE) == string(N->SEGMENT_SIZE, 0)){
//             N->fully_covered_counts[input_left_MSB] --;
//         }
//         if(input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE) == string(N->SEGMENT_SIZE, 255)){
//             N->fully_covered_counts[input_right_MSB] --;
//         }
//     }

//     // update left_boundary and right_boundary
//     if(N->left_boundary == input_left_MSB){
//         int i = N->left_boundary;
//         for(; i < N->right_MSB; i++){
//             if(N->fully_covered_counts[i] != 0){
//                 break;
//             }
//         }
//         N->left_boundary = i;
//     }
//     if(N->right_boundary == input_right_MSB){
//         int i = N->right_boundary;
//         for(; i > N->left_MSB; i--){
//             if(N->fully_covered_counts[i] != 0){
//                 break;
//             }
//         }
//         N->right_boundary = i;
//     }

//     //recursively remove
//     unsigned long idx_left = left_MSB;
//     unsigned long idx_right = right_MSB;

//     if(input_left_MSB_str.size() >= N->SEGMENT_SIZE){
//         // remove input_left and input_right falls in the same sub-region
//         if(idx_left == idx_right){
//             unsigned long idx = idx_left;
            
//             remove_range(N->childs[idx], input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE), 
//                 input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE));
        
//             return;
//         }

//         //remove input_left
//         remove_range(N->childs[idx_left], input_left.substr(N->SEGMENT_SIZE, input_left.size() - N->SEGMENT_SIZE), 
//             string(N->SEGMENT_SIZE, 255));

//         //remove input_right
//         remove_range(N->childs[idx_right], string(N->SEGMENT_SIZE, 0),
//             input_right.substr(N->SEGMENT_SIZE, input_right.size() - N->SEGMENT_SIZE));
        
//         return;
//     }

//     N->fully_covered_counts[idx_left] --;
//     N->fully_covered_counts[idx_right] --;
// }

// const int ELEMENT_LENGTH = 16; // bytes

// // Node *root = new Node(string(ELEMENT_LENGTH, 0), string(ELEMENT_LENGTH, 255));
// // Node *root = new Node(string(Node::SEGMENT_SIZE, 0), string(Node::SEGMENT_SIZE, 255));
// Node *root = new Node();


// using pstr2 = pair<string, string>;
// vector<pstr2> V;

// bool search_pq_ref(vector<pstr2> V, string input){
//     for(auto p : V){
//         if(p.first <= input && input <= p.second){
//             return false;
//         }
//     }
//     return true;
// }

void check_bracket_functions(){
    Node *root = new Node();
    vector<unsigned long> V_check1(root->right_MSB+1, 0);
    vector<pair<unsigned long, unsigned long>> V_ranges;

    int epoch = 30;
    while(epoch--){
        V_ranges.clear();

        //insert ranges
        for(int i = 0; i < 20; i++){
            unsigned long a = rand() % (root->right_MSB - root->left_MSB) + root->left_MSB;
            unsigned long b = rand() % (root->right_MSB - root->left_MSB) + root->left_MSB;

            for(int j = min(a,b); j <= max(a,b); j++){
                V_check1[j] ++;
            }

            V_ranges.push_back(make_pair(min(a,b), max(a,b))); 
            root->increase_left_bracket_count(root, min(a,b));
            root->increase_right_bracket_count(root, max(a,b));
        
// cout << "range = " << "[" << min(a,b) << ", " << max(a,b) << "]" << endl;    
// Node::walk_through_bracket_count(root);  
            for(int j = 0; j <= root->right_MSB; j++){
                if(root->is_point_covered_by_brackets(root, j)){
                    if(V_check1[j] == 0){
                        cout << "Error: should be blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }else{
                    if(V_check1[j] > 0){
                        cout << "Error: should not blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }
            }
        }

        //shuffle ranges
        std::random_device rd;
        std::mt19937 g(rd());
        random_shuffle(V_ranges.begin(), V_ranges.end());

        //remove ranges
        for(int i = 0; i < V_ranges.size(); i++){
            unsigned long a = V_ranges[i].first;
            unsigned long b = V_ranges[i].second;

            for(int j = min(a,b); j <= max(a,b); j++){
                V_check1[j] --;
            }
            root->decrease_left_bracket_count(root, min(a,b));
            root->decrease_right_bracket_count(root, max(a,b));

            for(int j = 0; j <= root->right_MSB; j++){
                if(root->is_point_covered_by_brackets(root, j)){
                    if(V_check1[j] == 0){
                        cout << "Error: should be blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }else{
                    if(V_check1[j] > 0){
                        cout << "Error: should not blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }
            }
        }


    }
}


void check_subrange_count_functions(){

    Node *root = new Node();
    // vector<pair<unsigned long, unsigned long>> V_check1;
    vector<unsigned long> V_check1(root->right_MSB+1, 0);
    vector<pair<unsigned long, unsigned long>> V_ranges;
    int epoch = 30;
    while(epoch--){
        V_ranges.clear();

        //insert ranges
        for(int i = 0; i < 20; i++){
            unsigned long a = rand() % (root->right_MSB - root->left_MSB) + root->left_MSB;
            unsigned long b = rand() % (root->right_MSB - root->left_MSB) + root->left_MSB;

            for(int j = min(a,b); j <= max(a,b); j++){
                V_check1[j] ++;
            }
            V_ranges.push_back(make_pair(min(a,b), max(a,b))); 
            root->increase_subrange_count(root, root->left_MSB, root->right_MSB, min(a,b), max(a,b));
            
// Node::walk_through_subrange_count(root);
            for(int j = (int) root->left_MSB; j <= root->right_MSB; j++){
                if(root->is_covered_by_subrange(root, root->left_MSB, root->right_MSB, j)){
                    if(V_check1[j] == 0){
                        cout << "Error: should be blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }else{
                    if(V_check1[j] > 0){
                        cout << "Error: should not blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }     
            }
        }

        //shuffle ranges
        std::random_device rd;
        std::mt19937 g(rd());
        
        // shuffle(V_ranges.begin(), V_ranges.end(), g);
        random_shuffle(V_ranges.begin(), V_ranges.end());
 
        //remove ranges
        for(int i = 0; i < V_ranges.size(); i++){
            unsigned long a = V_ranges[i].first;
            unsigned long b = V_ranges[i].second;

            for(int j = min(a,b); j <= max(a,b); j++){
                V_check1[j] --;
            }
            root->decrease_subrange_count(root, root->left_MSB, root->right_MSB, min(a,b), max(a,b));
            
// cout << "range = " << "[" << min(a,b) << ", " << max(a,b) << "]" << endl;
            for(int j = 0; j <= root->right_MSB; j++){
                if(root->is_covered_by_subrange(root, root->left_MSB, root->right_MSB, j)){
                    if(V_check1[j] == 0){
                        cout << "Error: should be blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }else{
                    if(V_check1[j] > 0){
                        cout << "Error: should not blocked" << " " << j << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                        assert(false);
                    }
                }
            }
        }

    }

}


void check_data_structure_function(){
    Node *root = new Node();
    vector<pair<string, string>> V_ranges;

    // vector<Node::elementType> a = Node::string2BitArray("00100", Node::N_BITS_PER_ELEMENT);
    // vector<Node::elementType> b = Node::string2BitArray("000111", Node::N_BITS_PER_ELEMENT);
    int epoch = 300;
    while(epoch--){
        V_ranges.clear();
        delete root;
        root = new Node();

        //insert ranges
        for(int i = 0; i < 20; i++){
            string a = "";
            string b = "";
            int n = rand() % 8 + 4;
            while(n--){
                a += (char) (rand() % 2) + '0';
            }
            n = rand() % 8 + 4;
            while(n--){
                b += (char) (rand() % 2) + '0';
            }
            if(a>b){swap(a,b);}

// cout << "a = " << a <<  " ,b = " << b << endl;

            vector<Node::elementType> a2 = Node::string2BitArray(a, Node::N_BITS_PER_ELEMENT);
            vector<Node::elementType> b2 = Node::string2BitArray(b, Node::N_BITS_PER_ELEMENT);


            for(int i = a.size()-1; i >= 0; i--){
                if(a[i] != 0){
                    break;
                }
                a.pop_back();
            }
            for(int i = b.size()-1; i >= 0; i--){
                if(b[i] != 0){
                    break;
                }
                b.pop_back();
            }
            V_ranges.push_back(make_pair(min(a,b), max(a,b))); 
            Node::insert_brackets(root, a2, b2);

            vector<string> V_c;
            for(int j = 0; j < 10; j++){
                string c = "";
                n = rand() % 8 + 4;
                while(n--){
                    c += (char) (rand() % 2) + '0';
                }
                V_c.emplace_back(c);
            }

            for(auto &c: V_c){
// cout << "c = " << c << endl;
                vector<Node::elementType> c2 = Node::string2BitArray(c, Node::N_BITS_PER_ELEMENT);



                bool rst1 = Node::is_covered_by_brackets(root, c2);
                bool rst2 = false;
                for(auto &x: V_ranges){
// cout << "x_range = " << x.first << " " << x.second << endl;
                    if(x.first <= c && c <= x.second){
                        rst2 = true;
                        break;
                    }
                }
                if(rst1 != rst2){
                    if(rst2 == true){
                        cout << "Error: should be blocked" << " " << c << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                    }else{
                        cout << "Error: should not blocked" << " " << c << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                    }
                    assert(false);
                }
            }   
        }

        //shuffle ranges
        std::random_device rd;
        std::mt19937 g(rd());
        random_shuffle(V_ranges.begin(), V_ranges.end());

        //delete ranges 
        for(int i = V_ranges.size()-1; i >= 0; i--){
            string a = V_ranges[i].first;
            string b = V_ranges[i].second;
            vector<Node::elementType> a2 = Node::string2BitArray(a, Node::N_BITS_PER_ELEMENT);
            vector<Node::elementType> b2 = Node::string2BitArray(b, Node::N_BITS_PER_ELEMENT);
            Node::delete_brackets(root, a2, b2);
            V_ranges.pop_back();

            vector<string> V_c;
            for(int j = 0; j < 10; j++){
                string c = "";
                int n = rand() % 8 + 4;
                while(n--){
                    c += (char) (rand() % 2) + '0';
                }
                V_c.emplace_back(c);
            }

            for(auto &c: V_c){
                vector<Node::elementType> c2 = Node::string2BitArray(c, Node::N_BITS_PER_ELEMENT);
                bool rst1 = Node::is_covered_by_brackets(root, c2);
                bool rst2 = false;
                for(auto &x: V_ranges){
// cout << "x_range = " << x.first << " " << x.second << endl;
                    if(x.first <= c && c <= x.second){
                        rst2 = true;
                        break;
                    }
                }
                if(rst1 != rst2){
                    if(rst2 == true){
                        cout << "Error: should be blocked" << " " << c << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                    }else{
                        cout << "Error: should not blocked" << " " << c << " " << __FILE__ << ":" << __LINE__ << " " << __FUNCTION__ << endl;
                    }
                    assert(false);
                }
            }
        }
    }
}

int main(){
    vector<Node::elementType> a = Node::string2BitArray("00100", Node::N_BITS_PER_ELEMENT);
    vector<Node::elementType> b = Node::string2BitArray("000111", Node::N_BITS_PER_ELEMENT);
    // vector<Node::elementType> a = Node::string2BitArray("00100", 4);
    // vector<Node::elementType> b = Node::string2BitArray("000111", 4);
    // for(auto e : a){
    //     cout << (int) e << " ";
    // }
    // cout << endl;
    // for(auto e : b){
    //     cout << (int) e << " ";
    // }
    // cout << endl;

    check_subrange_count_functions();
    check_bracket_functions();

    check_data_structure_function();
}