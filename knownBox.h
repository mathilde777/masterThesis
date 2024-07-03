#ifndef KNOWNBOX_H
#define KNOWNBOX_H
#include <string>
class KnownBox{
public:

    int productId;
    std::string productName;
    int new_box;
    int trained;


    KnownBox(int pId, const std::string& pName, int nBox, int trained)
        : productId(pId), productName(pName), new_box(nBox), trained(trained) {}


    void setProductId(int id) { this->productId = id; }
    void setProductName(const std::string& name) { this->productName = name; }
    void setNewBox(int nBox) { this->new_box = nBox; }
    void setTrained(int trained) { this->trained = trained; }


    int getProductId() const { return productId; }
    std::string getProductName() const { return productName; }
    int getNewBox() const { return new_box; }
    int getTrained() const { return trained; }
};
#endif // KNOWNBOX_H
