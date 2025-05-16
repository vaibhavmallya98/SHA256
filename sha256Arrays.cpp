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

//unique_ptr is a smart pointer which makes sure that memory is automatically freed when it's no longer needed.
//Prevents memory leaks and double deletes by handling cleanup for you.

std::unique_ptr<uint8_t []> messageToByteArray(std::string message){
    size_t messageSize = message.size(); 
    std::unique_ptr<uint8_t []> byteArray(new uint8_t[message.length() + 1]);
    
    std::memcpy(byteArray.get(), message.data(), messageSize);

    return byteArray; 
}


uint8_t* padInputMessage(uint8_t* inputMessage, size_t messageLength){
    if(messageLength <= 55){

        size_t paddedMessageLength = 64; 
        uint8_t* paddedMessage = new uint8_t[paddedMessageLength]; 
        std::copy(inputMessage, inputMessage + messageLength, paddedMessage);
        
        paddedMessage[messageLength] = 0x80; 

        std::fill(paddedMessage + messageLength + 1, paddedMessage + 56, 0);
        
        uint64_t messageBits = static_cast<uint64_t>(messageLength) * 8;
        for (int i = 0; i < 8; ++i) {
            paddedMessage[56 + i] = static_cast<uint8_t>((messageBits >> (56 - 8 * i)) & 0xFF);
        }

        return paddedMessage; 

    }
    else{
        size_t zeroBytes = (55 - messageLength)%64; 
        if(zeroBytes < 0){
            zeroBytes += 64; 
        }
        size_t paddedMessageLength = messageLength + 1 + zeroBytes + 8; 

        uint8_t* paddedMessage = new uint8_t[paddedMessageLength];
        
        std::copy(inputMessage, inputMessage + messageLength, paddedMessage);
        
        paddedMessage[messageLength] = 0x80; 

        std::fill(paddedMessage + messageLength + 1, paddedMessage + messageLength + 1 + zeroBytes, 0);
        
        uint64_t messageBits = static_cast<uint64_t>(messageLength) * 8;
        for (int i = 0; i < 8; ++i) {
            paddedMessage[messageLength + 1 + zeroBytes + i] = static_cast<uint8_t>((messageBits >> (messageLength + 1 + zeroBytes - 8 * i)) & 0xFF);
        }

        std::cout<<"\nPadded message (for arrays) is: ";
        for (const auto* it = paddedMessage; it != paddedMessage + messageLength + 1 + zeroBytes + 8; ++it) {
            std::cout << std::hex << static_cast<int>(*it) << " ";
        }

        
    }

    return nullptr; 
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

std::array<uint32_t,64> wordExpansionSingleBlock(uint8_t* paddedMessage){
    
    
    std::array<uint32_t,64>words; 

    //try to use iterators from C++ STL which are of the form iterator(s.begin(), s.end()) where s is a range 
    for(int i = 0; i < 16; i++){
        
        //std::bitset<32> stringBits(preprocessedMessage.substr(i*32,32) );
        words[i] = static_cast<uint32_t>(paddedMessage[i * 4] << 24) |
                   static_cast<uint32_t>(paddedMessage[i * 4 + 1] << 16) |
                   static_cast<uint32_t>(paddedMessage[i * 4 + 2] << 8) |
                   static_cast<uint32_t>(paddedMessage[i * 4 + 3]);
        // std::stringstream ssw;
        // ssw << std::hex << words[i]; // Convert to hexadecimal
        // std::cout <<"\nWord "<<i+1<<" = "<<ssw.str();
        //cout<<"\nWord "<<i+1<<" = "<<words[i];
    }

    //uint32_t temp; 

    for(int i = 16; i < 64; i++){
        words[i] = words[i - 16] + sigma0(words[i - 15]) + words[i - 7] + sigma1(words[i - 2]); 
    }
    
    // for(int i = 16; i < 64; i++){
    //     std::stringstream ss;
    //     ss << std::hex << words[i]; // Convert to hexadecimal
    //     std::cout <<"\nWord "<<i+1<<" = "<<ss.str();
        
    // }

    return words; 
}

//Correct output produced for single block of data 
std::array<uint32_t,8> computeSHA256SingleBlock(std::array<uint32_t,64> words, std::array<uint32_t,8> inputHashes){

    
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

// size_t paddedMessageLength = 64; 
// uint8_t* paddedMessage = new uint8_t[paddedMessageLength]; 
// std::copy(inputMessage, inputMessage + messageLength, paddedMessage);
// //Word expansion for multiple blocks 
std::vector<std::array<uint32_t,64>> wordExpansionMultipleBlocks(uint8_t* paddedMessage, int numBlocks){
    std::vector<std::array<uint32_t,64>> multiWordBlocks;
    multiWordBlocks.reserve(numBlocks);
    
    for(int b = 0; b < numBlocks; b++){
        //uint8_t* starttIndex = paddedMessage + b * 64; 
        
        //uint8_t* singleBlock = new uint8_t [64];
        std::array<uint8_t, 64> singleBlock;
        std::copy(paddedMessage + b * 64, paddedMessage + (b + 1) * 64, singleBlock.begin());
        //std::copy(starttIndex, starttIndex + 64, singleBlock); 

        auto words = wordExpansionSingleBlock(singleBlock.data()); 
        //delete[] singleBlock;

        //std::move(words.begin(), words.end(), std::back_inserter(multiWordBlocks.emplace_back()));
        multiWordBlocks.push_back(words); 

        
    }

     
    return multiWordBlocks; 
}

//Correct output produced for single block of data 
std::array<uint32_t,8> computeSHA256MultipleBlocks(std::vector<std::array<uint32_t,64>> multiWordBlocks, std::array<uint32_t,8> initialHashes){
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

std::string hashToHexUTF8(std::array<uint32_t,8>& hash) {
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
    //std::cout<<"\nOriginal message is ";
    std::string message = "System Software is Fun!";  
    auto start = std::chrono::high_resolution_clock::now(); 
    

    std::unique_ptr<uint8_t []> byteArray = messageToByteArray(message);
    size_t messageSize = message.size(); 

    //char* charArray = new char[message.length() + 1];
    //std::strcpy(charArray, message.c_str());

    uint8_t* arrUint8 = new uint8_t[messageSize]; 
    std::copy(message.begin(), message.end(), arrUint8);


    uint8_t* paddedMessage = padInputMessage(arrUint8, messageSize); 
    //std::array<uint32_t,64> wordsSingleBlock = smartWordExpansionSingleBlock(paddedMessage);


    int numBlocks = (messageSize/64) + 1; 


    std::cout<<"\nTotal number of blocks required = "<<numBlocks; 

    // std::unique_ptr<uint8_t []> paddedMessage = padMessage(byteArray, messageSize);

    std::array<uint32_t,8> initialHashArray =  {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

    if(messageSize <= 55){
        std::cout<<"\nPreprocessed Message is a single data block";
        std::array<uint32_t,64> wordsSingleBlock = wordExpansionSingleBlock(paddedMessage);
        std::array<uint32_t,8> finalHashSingleBlock = computeSHA256SingleBlock(wordsSingleBlock, initialHashArray); 
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - start; 
        std::string hexHashSingleBlock = hashToHexUTF8(finalHashSingleBlock);
        std::cout << "\nSHA-256 hash in hex (for arrays): " << hexHashSingleBlock << std::endl;
        std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;
    }
    else{
        std::cout<<"\nPreprocessed Message is a multiple data block";
        //int numBlocks = preprocessedMessage.size()/512; 
        //std::cout<<"\nPreprocessed Message contains "<<numBlocks<<" blocks"; 
        std::vector<std::array<uint32_t,64>> wordsMultipleBlocks = wordExpansionMultipleBlocks(paddedMessage, numBlocks);
        // std::array<uint32_t,8>hash = smartComputeSHA256MultipleBlocks(wordsMultipleBlocks, initialHashArray); 
        // auto end = std::chrono::high_resolution_clock::now();
        // std::chrono::duration<double, std::milli> duration = end - start; 

        // std::string hexHash = hashToHexUTF8(hash);
        // std::cout << "\nSHA-256 hash in hex: " << hexHash << std::endl;
        // std::cout << "Time taken: " << duration.count() << " milliseconds" << std::endl;

    }

    std::cout<<"\n"; 

    delete[] arrUint8; 
    delete[] paddedMessage; 
    return 0; 

}