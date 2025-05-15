#include<iostream>
#include<cstdint> 
#include<bitset> 
#include<string> 
#include<vector>
#include<algorithm> 
#include<sstream>
#include<iomanip> 
#include<chrono> 
#include<string_view> 
#include<cstring>
#include<memory>


//using namespace std; 

//Function to convert message to binary 

//use this function to return a character array 
std::string messageToBinary(std::string_view message){

    std::string binaryString; 
    for(char c: message){
        std::bitset<8> charBits (static_cast<unsigned char>(c));
        //cout<<"\nCharacter bits for "<<c<<" = "<<charBits;  
        binaryString.append(charBits.to_string());
    }
    return binaryString; 

}


//Function to append one at the end of the message 
std::string appendOne(std::string_view binaryString){
    std::string result(binaryString); 
    result.append(1,'1'); 
    return result; 
}

//Function to pad zeros if length of message is less than 448 bit 

//try using character arrays instead of std::string as std::string is inefficient 
std::string padZeros(std::string_view binaryString){
    std::string result(binaryString); 
    size_t messageLength = binaryString.size(); 

    //for a message which is less than 448 bits in length 
    if(messageLength <= 448){
        //int numZeros = 448 - messageLength; 
        //cout<<"\nNumber of zeros to be padded = "<<numZeros; 
        //std::string paddedZeros(numZeros,'0');
        result.append(448 - messageLength,'0');
        //cout<<"\nLength of padded zero string = "<<paddedZeros.size(); 
        //binaryString = binaryString + paddedZeros; 

    }
    //for multiple blocks 
    else{
        int numZeros = (449 - (messageLength + 1)) % 512;
        if (numZeros < 0){
            numZeros += 512;
        }
        
        //cout<<"\nNumber of zeros to be padded = "<<numZeros; 
        //std::string paddedZeros(numZeros,'0'); 
        result.append(numZeros,'0'); 
        //cout<<"\nLength of padded zero string = "<<paddedZeros.size(); 
        //binaryString += paddedZeros; 
    }
    return result;
}



//Function to store the length of the message as 64 bit 
std::string preprocessedString(std::string_view message){

    //string message_copy = message; 
    std::string preprocessedMessage;  
    std::string binaryString = messageToBinary(message); 
    uint64_t stringLength = binaryString.size(); 
    //cout<<"\nLength of binary string = "<<stringLength;
    std::bitset<64> lengthBits(stringLength); 
    std::string stringLengthAsString = lengthBits.to_string(); 
    //cout<<"\nString length as bits = "<<lengthBits;
    //cout<<"\nString length as string = "<<stringLengthAsString; 
    //cout<<"\nLength of string length as string = "<<stringLengthAsString.size();

    binaryString = appendOne(binaryString); 
    //cout<<"\nBinary string with 1 appended = "<<binaryString; 
    binaryString = padZeros(binaryString); 
    
    preprocessedMessage = binaryString + stringLengthAsString;  
    
    return preprocessedMessage; 
}



uint32_t ROTR(uint32_t word, int n){
    uint32_t result = (word >> n) | (word << 32 - n); 
    return result;
}

uint32_t sigma0(uint32_t word){
    uint32_t result = ROTR(word, 7) ^ ROTR(word, 18) ^ (word >> 3); 
    return result; 

}

uint32_t sigma1(uint32_t word){
    uint32_t result = ROTR(word, 17) ^ ROTR(word, 19) ^ (word >> 10); 
    return result; 

}


//Word expansion is producing the correct output 

std::vector<uint32_t> wordExpansionSingleBlock(std::string preprocessedMessage){
    
    
    std::vector<uint32_t>words(64); 
    

    //try to use iterators from C++ STL which are of the form iterator(s.begin(), s.end()) where s is a range 
    for(int i = 0; i < 16; i++){
        
        std::bitset<32> stringBits(preprocessedMessage.substr(i*32,32) );
        words[i] = static_cast<uint32_t>(stringBits.to_ulong());
        //std::stringstream ssw;
        //ssw << std::hex << words[i].to_ullong(); // Convert to hexadecimal
        //std::cout <<"\nWord "<<i+1<<" = "<<ssw.str();
        //cout<<"\nWord "<<i+1<<" = "<<words[i];
    }

    //uint32_t temp; 

    for(int i = 16; i < 64; i++){
        words[i] = words[i - 16] + sigma0(words[i - 15]) + words[i - 7] + sigma1(words[i - 2]); 
    }
    
    // for(int i = 16; i < 64; i++){
    //     std::stringstream ss;
    //     ss << std::hex << words[i].to_ullong(); // Convert to hexadecimal
    //     std::cout <<"\nWord "<<i+1<<" = "<<ss.str();
    //     //cout<<"\nWord "<<i+1<<" = "<<words[i];
    // }

    return words; 
}

//Correct output produced for single block of data 
std::vector<uint32_t> computeSHA256SingleBlock(std::vector<uint32_t> words, std::vector<uint32_t> inputHashes){

    
    uint32_t S1; 
    uint32_t S0;
    uint32_t ch; 
    uint32_t maj;
    uint32_t temp1, temp2; 

    //round constants k0 to k63 
    //declare round constants as a const std::array with a fixed size as it reduces the overhead of dynamically allocated vectors
    //std::array is statically allocated on the stack 
    const std::array<uint32_t,64> k = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
        0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
        0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
        0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
        0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
        0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
        0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
        0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
        0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

    //initialized a,b,c,d,e,f,g,h 
    uint32_t a_hash = inputHashes[0];
    uint32_t b_hash = inputHashes[1]; 
    uint32_t c_hash = inputHashes[2]; 
    uint32_t d_hash = inputHashes[3];
    uint32_t e_hash = inputHashes[4]; 
    uint32_t f_hash = inputHashes[5];
    uint32_t g_hash = inputHashes[6]; 
    uint32_t h_hash = inputHashes[7];

    for(int i = 0; i < 64; i++){

        S1 = ROTR(e_hash, 6) ^ ROTR(e_hash, 11) ^ ROTR(e_hash, 25); 
        ch = (e_hash & f_hash) ^ ((~e_hash) & g_hash); 
        temp1 = h_hash + S1 + ch + k[i] + words[i]; 
        S0 = ROTR(a_hash, 2) ^ ROTR(a_hash, 13) ^ ROTR(a_hash, 22); 
        maj = (a_hash & b_hash) ^ (b_hash & c_hash) ^ (c_hash & a_hash); 
        temp2 = S0 + maj; 

        h_hash = g_hash; 
        g_hash = f_hash; 
        f_hash = e_hash; 
        e_hash = d_hash + temp1;
        d_hash = c_hash; 
        c_hash = b_hash; 
        b_hash = a_hash; 
        a_hash = temp1 + temp2;  

    }

    inputHashes[0] += a_hash;
    inputHashes[1] += b_hash;
    inputHashes[2] += c_hash;
    inputHashes[3] += d_hash;
    inputHashes[4] += e_hash;
    inputHashes[5] += f_hash;
    inputHashes[6] += g_hash; 
    inputHashes[7] += h_hash;

    return inputHashes; 

}

//Word expansion for multiple blocks 
std::vector<std::vector<uint32_t>> wordExpansionMultipleBlocks(std::string preprocessedMessage){
    
    int numBlocks = preprocessedMessage.size()/512; 
    //int preprocessedMessageLength = preprocessedMessage.size();
    std::vector<std::vector<uint32_t>> multiWordBlocks;

    /*
    vector::reserve reserves space for numBlocks elements upfront in the multiWordBlocks vector. 

    This results in only one memory allocation.

    There are no reallocations or internal copying.

    So everytime we do push_back it doesn't have to allocate memory for each element
    */

    multiWordBlocks.reserve(numBlocks);
    
    for(int b = 0; b < numBlocks; b++){

        auto words = wordExpansionSingleBlock(preprocessedMessage.substr(b*512,512)); 

        //multiWordBlocks.push_back(std::move(words));

        /*
        This creates a new empty std::vector<uint32_t> inside multiWordBlocks and returns a reference to it.
        emplace_back() constructs the object in-place, avoiding an extra copy or move.
        std::back_inserter creates a back-inserting iterator to the just-created inner vector.
        It allows std::move to insert elements into this vector as if you're doing vec.push_back() for each element.
        std::move moves (not copies) each element from words into the inner vector created in multiWordBlocks.

        Why is this efficient?
        1. No unnecessary temporary vectors.

        2. No redundant constructor/destructor calls.

        3. Optimal memory and time usage.

        */
        std::move(words.begin(), words.end(), std::back_inserter(multiWordBlocks.emplace_back()));

    }

    return multiWordBlocks; 
}


//Correct output produced for single block of data 
std::vector<uint32_t> computeSHA256MultipleBlocks(std::vector<std::vector<uint32_t>> multiWordBlocks, std::vector<uint32_t> initialHashes){
    /*
    const auto& block in the loop avoids copying each block.
    express intent of looping over the elements of multiwordblocks using const auto& block 
    */
    for(const auto & block: multiWordBlocks){

        //auto newHashes = computeSHA256SingleBlock(block, initialHashes);

        //std::move() efficiently transfers ownership of the result into initialHashes, avoiding an unnecessary copy.
        initialHashes = std::move(computeSHA256SingleBlock(block, initialHashes)); 

    }
    
    return initialHashes; 

}

std::string hashToHexUTF8(const std::vector<uint32_t>& hash) {
    std::ostringstream oss;
    for (uint32_t word : hash) {
        // Output each 32-bit word as 8 hex digits (big-endian)
        oss << std::hex << std::setfill('0')
            << std::setw(2) << ((word >> 24) & 0xFF)
            << std::setw(2) << ((word >> 16) & 0xFF)
            << std::setw(2) << ((word >> 8) & 0xFF)
            << std::setw(2) << (word & 0xFF);
    }
    return oss.str();
}

int main(){

    std::string message = "Hello Earthlings! It's me Vaibhav speaking to you from Mindgrove. I am happy to be back. WOOHOOO!!!!! System Software Engineering is fun! I am excited about this new job! I successfully got SHA256 to work for multiple blocks! IITM RP is full of intelligent and hard working people. We are currently working on the third chip. It will called as Vision SoC. I will be working towards providing software support for the chip.";
    //std::string message = "System Software is Fun!";  
    auto start = std::chrono::high_resolution_clock::now();
    std::string preprocessedMessage = preprocessedString(message); 
    auto endPadding = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> durationpad = endPadding - start;
    std::cout << "\nTime taken for string to pad: " << durationpad.count() << " milliseconds" << std::endl;
    //std::cout<<"\nPreprocessed Message Binary = "<<preprocessedMessage; 
    //std::cout<<"\nLength of preprocessed message = "<<preprocessedMessage.size(); 

    //initial hashes are constants 
    std::vector<uint32_t> initialHashes =  {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    if(preprocessedMessage.size() == 512){
        std::cout<<"\nPreprocessed Message is a single data block";
        std::vector<uint32_t>words = wordExpansionSingleBlock(preprocessedMessage);         
        std::vector<uint32_t> finalHash = computeSHA256SingleBlock(words, initialHashes); 
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start; 
        std::string hexHash = hashToHexUTF8(finalHash);
        std::cout << "\nSHA-256 hash in hex (for vectors and strings): " << hexHash << std::endl;
        std::cout << "Execution Time taken (for vectors and strings): " << duration.count() << " milliseconds" << std::endl;
    }
    else{
        std::cout<<"\nPreprocessed Message is a multiple data block";
        int numBlocks = preprocessedMessage.size()/512; 
        std::cout<<"\nPreprocessed Message contains "<<numBlocks<<" blocks"; 
        std::vector<std::vector<uint32_t>>wordBlocks = wordExpansionMultipleBlocks(preprocessedMessage);
        std::vector<uint32_t>hash = computeSHA256MultipleBlocks(wordBlocks, initialHashes); 
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start; 


        std::string hexHash = hashToHexUTF8(hash);
        std::cout << "\nSHA-256 hash in hex (for vectors and strings): " << hexHash << std::endl;
        std::cout << "Execution Time taken (for vectors and strings): " << duration.count() << " milliseconds" << std::endl;

    }

    std::cout<<"\n"; 
    return 0; 

}