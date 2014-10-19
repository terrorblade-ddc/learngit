/*
  Name: SALES TAXES
  Author: DUAN DE CHANG
  Date: 10/10/14 21:54
  Description:
        an application that prints out the receipt details for these shopping baskets
  Caution:
        1. our assumption of the count and price of is described in the functions of
           "isvalid_count" and "isvalid_price".
        2. we use two libraries to judge whether an item is imported and free of basic tax,
           they are stored in the following files: "lib.txt" and "prep_lib.txt"
        3. the input is stored in file "in.txt"
*/
#include <iostream>
#include <fstream>
#include <string>
#include <map>

using namespace std;

typedef unsigned int uint;

#define BASIC_SALE 0.1
#define IMPORTED_SALE 0.05
#define MAX_CHARACTER 200
#define DELIMS " "
#define INPUT_TITLE "input"
#define OUTPUT_TITLE "Output"
#define INPUT_DATA "in.txt"
#define BASIC_LIB "lib.txt"
#define PREP_LIB "prep_lib.txt"

class ComputeSales{
public:
    ComputeSales(){
        total_tax = total_price = 0.0;
    }
    
    // invoke this function to compute the output of INPUT_DATA
    void compute(){
        //============= load words libraries =============
        if(!load_lib(BASIC_LIB, lexicon_lib_mp) ||
           !load_lib(PREP_LIB, prep_lib_mp)) return;
        
        //============= load and deal with input =============
        ifstream fin(INPUT_DATA);
        if(!open_file(fin, INPUT_DATA)) return;
        
        char line[MAX_CHARACTER] = "\0";
        char tline[MAX_CHARACTER] = "\0";
        int headnum = 0, headvalue = 0;
        while(fin.getline(line, MAX_CHARACTER)){
            // in case of an empty line
            if('\0' == line[0]) continue;
            strncpy(tline, line, MAX_CHARACTER);
            if(is_headline(line, headvalue)){
                char output[MAX_CHARACTER] = "\0";
                sprintf(output, "%i", headvalue);
                
                if(0 < headnum) output_total();
                cout << OUTPUT_TITLE << " " << output << ":" << endl;
                headnum++;
            }else if(0 < headnum){
                compute_item(tline);
            }else{
                illegal_item();
            }
            
            memset(line, '\0', MAX_CHARACTER);
        }
        // output the summary of last part
        if(0 < headnum) output_total();
        
        fin.close();
    }
    
private:
    // judge whether item_str is a "input 1:" format string
    // and return the correct input number
    bool is_headline(char* item_str, int& val){
        map<int, string> mp;
        int idx = 0;
        
        // store all the words in mp
        split2map(item_str, DELIMS, idx, mp);
        if(2 != mp.size()) return false;
        
        map<int, string>::iterator iter = mp.begin();
        str2lower(iter->second);
        if(INPUT_TITLE == iter->second){
            iter++;
            uint size = iter->second.size();
            string order = iter->second.substr(0, size - 1);
            val = isvalid_count(order.c_str());
            if(1 < size && 0 < val) return true;
        }
        return false;
    }
    
    // judge whether item_str is a "1 chocolate bar at 0.85" format string
    // and output the corresponding result
    void compute_item(char* item_str){
        map<int, string> mp;
        int total, idx = 0;
        
        uint _count = 0;
        double _price = 0.0;
        string _info = "";
        double _sale = 0.0;
        bool b_is_basic = false;      // 0.1
        bool b_is_imported = false;   // 0.05
        
        //================= part 1 =================
        // store all the words in mp
        split2map(item_str, DELIMS, idx, mp);
        total = idx;
        idx = 0;
        
        //================= part 2 =================
        // 1. judge the type of item: is_basic\is_imported
        // 2. get count and price of item
        map<int, string>::iterator iter;
        for(iter = mp.begin(); iter != mp.end(); iter++){
            b_is_basic = b_is_basic == true ? true : is_basic(iter->second.c_str());
            b_is_imported = b_is_imported == true ? true : is_imported(iter->second.c_str());
            
            if(0 == idx){
                _count = isvalid_count(iter->second.c_str());
                if(0 == _count){
                    illegal_item();
                    return;
                }
            }else if(idx + 2 < total){
                if("" != _info) _info += " ";
                _info += iter->second;
            }else if(idx + 1 == total){
                _price = isvalid_price(iter->second.c_str());
                if(0 == _price){
                    illegal_item();
                    return;
                }
            }
            idx++;
        }
        
        //================= part 3 =================
        if(!b_is_basic) _sale += BASIC_SALE;
        if(b_is_imported) _sale += IMPORTED_SALE;
        // compute sale
        add_sale(_price, _count, _sale);
        
        //================= part 4 =================
        printf("%i %s: %.2f\n", _count, _info.c_str(), _price);
    }
    
    // compute sale
    // add it to orginal and total price
    inline void add_sale(double& _price, uint& _count, double& _sale){
        double tmp_sale = _sale * _price;
        regularize_price(tmp_sale);
        
        _price += tmp_sale;
        // in case of _price = 1.858, we should output 1.85, not 1.86
        more_than_2_decimal(_price);
        
        total_price += _price;
        total_tax += tmp_sale;
    }
    
    // in case of _price = 1.858, we should output 1.85, not 1.86
    inline void more_than_2_decimal(double& _price, int is_sale){
        char sprice[100] = "\0";
        sprintf(sprice, "%.3f", _price);
        int i = 0;
        while(sprice[i] != '\0') i++;
        i--;
        if(is_sale) regularize_price();
        else sprice[i] = '\0';
        
        char* tmp = new char[i + 2];
        memset(tmp, '\0', i + 2);
        strncpy(tmp, sprice, i + 2);
        // convert string to number
        _price = isvalid_price(tmp);
        
        delete[] tmp;
    }
    
    inline void illegal_item(){
        printf("Illegal item\n");
    }
    
    inline void output_total(){
        printf("Sales Taxes: %.2f\n", total_tax);
        printf("Total: %.2f\n", total_price);
        // reset to zero
        total_tax = total_price = 0.0;
    }
    
     // round the _price as the following manners:
    // 1. 1.204 -> 1.20, 1.234 -> 1.25, 1.254 -> 1.25, 1.264 -> 1.29
    // 2. 1.209 -> 1.25, 1.239 -> 1.25, 1.254 -> 1.26, 1.269 -> 1.27
    inline void regularize_price(char* sprice, int& i, double& carry){
        if('0' != sprice[i]){
            if(sprice[i] < '5') sprice[i] = '5';
            else{
                carry = 0.1;
                sprice[i] = '0';
            }
        }
    }
    
    /*
    // round the _price as the following manners:
    // 1. 1.204 -> 1.20, 1.234 -> 1.25, 1.254 -> 1.25, 1.264 -> 1.29
    // 2. 1.209 -> 1.25, 1.239 -> 1.25, 1.254 -> 1.26, 1.269 -> 1.27
    inline void regularize_price(double& _price){
        char sprice[100] = "\0";
        // !caution: sprintf will make rounding operation
        // when we use the format string, such as "%.2f"
        sprintf(sprice, "%.2f", _price);
        
        double carry = 0.0;
        int i = 0;
        while(sprice[i] != '\0') i++;
        i--;
        if('0' != sprice[i]){
            if(sprice[i] < '5') sprice[i] = '5';
            else{
                carry = 0.1;
                sprice[i] = '0';
            }
        }
        
        char* tmp = new char[i + 2];
        memset(tmp, '\0', i + 2);
        strncpy(tmp, sprice, i + 2);
        // convert string to number
        _price = isvalid_price(tmp);
        _price += carry;
        
        delete[] tmp;
    }
    */
    
    // judge where count of item is a correct number, 
    // which satisfy the following conditions:
    // 1. +123, 123
    // 2. 0 is a illegal number
    inline uint isvalid_count(const char* count_str){
        uint err = 0;
        if(NULL == count_str) return err;
        
        uint length = strlen(count_str);
        uint i = 0, icount = 0;
        
        if('+' == count_str[0]) i++;
        // "count_str" is a single character string with '+'
        if(i == length) return err;
        
        for(; i < length; i++){
            if(!isdigit(count_str[i])) return err;
            icount = icount * 10 + (count_str[i] - '0');
        }
        return icount;
    }
    
    // judge where _price represent the correct number, 
    // which satisfy the following conditions:
    // 1. +123.2, +0123.2, +0.123
    // 2. +1+23, -1.2, .123, 123., 1.2.3 are illegal numbers 
    inline double isvalid_price(const char* price_str){
        double err = 0.0;
        if(NULL == price_str) return err;
        
        uint length = strlen(price_str);
        int sign = 0, point = 0;
        double price = 0.0;
        uint i = 0;
        char ch;
        
        for(i = 0; i < length; i++){
            ch = price_str[i];
            if('+' == ch){
                if(!sign && 0 == i) sign = 1;
                else return err;
            }else if('.' == ch){
                int start = 0;
                if(0 != sign) start++;
                if(!point && (start < i && i < length - 1)) point = length - i - 1;
                else return err;
            }else if(isdigit(ch)){
                price = price * 10 + (ch - '0');
            }else{
                return err;
            }
        }
        
        while(point-- > 0) price /= 10;
        
        return price;
    }
    
    inline bool is_imported(const char* str){
        string sstr(str);
        str2lower(sstr);
        return ("imported" == sstr || "import" == sstr) ? true : false;
    }
    
    // basic word should obey the following rules:
    // 1. not belong to "prep_lib_mp"
    // 2. belong to lexicon_lib_mp
    inline bool is_basic(const char* str){
        string sstr(str);
        str2lower(sstr);
        return (!search_lib(sstr, prep_lib_mp) && 
               search_lib(sstr, lexicon_lib_mp)) ? true : false;
    }
    
    bool load_lib(char* file, map<string, int>& mp){
        ifstream fin(file);
        if(!open_file(fin, file)) return false;
        
        char line[MAX_CHARACTER] = "\0";
        int i = 0; 
        while(fin.getline(line, MAX_CHARACTER)){
            mp[string(line)] = i++;
            memset(line, '\0', MAX_CHARACTER);
        }
        fin.close();
        
        return true;
    }
    
    // judge whether there the "mp" contains "info" such as:
    // 1. "abc" = "abc"
    // 2. "abc def" = "abc"
    // 3. "abc def" = "def"
    // 4. "abc def ghi" = "def"
    inline bool search_lib(string info, map<string, int>& mp){
        map<string, int>::iterator iter;
        for(iter = mp.begin(); iter != mp.end(); iter++){
            uint loc = iter->first.find(info, 0);
            if(string::npos == loc) continue;
            if(iter->first.size() == info.size() ||                                     // "abc" = "abc"
               (0 == loc && ' ' == iter->first[loc + info.size()]) ||                   // "abc def" = "abc"
               (iter->first.size() - 1 == loc && ' ' == iter->first[loc - 1]) ||        // "abc def" = "def"
               (' ' == iter->first[loc - 1] && ' ' == iter->first[loc + info.size()])){ // "abc def ghi" = "def"
                return true;
            }
        }
        return false;
    }
    
    // split "item_str" with delims and store the words in mp
    inline void split2map(char* item_str, char* delims, int& idx, map<int, string>& mp){
        char* result = strtok(item_str, delims);
        while(result != NULL) {
            mp[idx++] = string(result);
            result = strtok(NULL, delims);
        }
    }
    
    inline void str2lower(string& item){
        int i = item.size();
        while(i-- >= 0) item[i] = tolower(item[i]);
    }
    
    inline bool open_file(ifstream& fin, char* path){
        if(!fin) {
            cout << "Error opening input file: " << path << endl;
            return false;
        }
        return true;
    }
    
    map<string, int> lexicon_lib_mp;
    map<string, int> prep_lib_mp;
    double total_price;
    double total_tax;
};

int main(){
    ComputeSales sale;
    sale.compute();
    
    system("pause");
    return 0;
}
